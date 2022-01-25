#include <BluetoothManager.h>

#include <system_info.h>
#include <Logger.h>
#include <LogLevel.h>

#include <bluetooth.h>
#include <tizen.h>

#include <BluetoothDeviceController.h>


namespace btu{
    using LogLevel = btlog::LogLevel;
    using Logger = btlog::Logger;
    auto decodeAdvertisementData(char* packetsData, proto::gen::AdvertisementData& adv, int dataLen) -> void;


    BluetoothManager::BluetoothManager(NotificationsHandler& notificationsHandler) noexcept:
    _notificationsHandler(notificationsHandler){
        if(!isBLEAvailable()){
            Logger::log(LogLevel::ERROR, "Bluetooth is not available on this device!");
            return;
        }
        if(bt_initialize()){
            Logger::log(LogLevel::ERROR, "[bt_initialize] failed");
            return;
        }
        if(bt_gatt_set_connection_state_changed_cb(&BluetoothDeviceController::connectionStateCallback, this)) {
            Logger::log(LogLevel::ERROR, "[bt_device_set_bond_destroyed_cb] failed");
            return;
        }
        Logger::log(LogLevel::DEBUG, "All callbacks successfully initialized.");
    }

    BluetoothManager::~BluetoothManager() noexcept{

    }

    auto BluetoothManager::isBLEAvailable() noexcept -> bool{
        bool state;
        int ret = system_info_get_platform_bool("http://tizen.org/feature/network.bluetooth.le", &state);
        if(ret != SYSTEM_INFO_ERROR_NONE){
            Logger::log(LogLevel::ERROR, "failed fetching bluetooth data!");
        }
        return state;
    }

    auto BluetoothManager::bluetoothState() const noexcept -> proto::gen::BluetoothState{
        /* Checks whether the Bluetooth adapter is enabled */
        bt_adapter_state_e adapter_state;
        int ret = bt_adapter_get_state(&adapter_state);
        proto::gen::BluetoothState bts;
        if(ret == BT_ERROR_NONE){
            if(adapter_state == BT_ADAPTER_ENABLED)
                bts.set_state(proto::gen::BluetoothState_State_ON);
            else
                bts.set_state(proto::gen::BluetoothState_State_OFF);
        }
        else if(ret == BT_ERROR_NOT_INITIALIZED)
            bts.set_state(proto::gen::BluetoothState_State_UNAVAILABLE);
        else
            bts.set_state(proto::gen::BluetoothState_State_UNKNOWN);
        
        return bts;
    }

    auto BluetoothManager::adapterStateChangedCallback(int result, bt_adapter_state_e adapter_state, void* user_data) noexcept -> void {
        BluetoothManager& bluetoothManager = *static_cast<BluetoothManager*> (user_data);
        std::scoped_lock lock(bluetoothManager.adapterState.mut);
        bluetoothManager.adapterState.var = adapter_state;
    }
    
    auto BluetoothManager::startBluetoothDeviceScanLE(const proto::gen::ScanSettings& scanSettings) noexcept -> void {
        std::scoped_lock l(_bluetoothDevices.mut, _scanAllowDuplicates.mut);
        _bluetoothDevices.var.clear();        
        _scanAllowDuplicates.var=scanSettings.allow_duplicates();        
        Logger::log(LogLevel::DEBUG, "allowDuplicates="+std::to_string(_scanAllowDuplicates.var));

        int res = bt_adapter_le_set_scan_mode(BT_ADAPTER_LE_SCAN_MODE_BALANCED);
        //remove from bluetooth Devices all devices with status==Scanned

        int uuidCount=scanSettings.service_uuids_size();
        std::vector<bt_scan_filter_h> filters(uuidCount);

        for(int i=0;i<uuidCount;i++){
            const std::string& uuid=scanSettings.service_uuids()[i];
            bt_adapter_le_scan_filter_create(&filters[i]);
            bt_adapter_le_scan_filter_set_device_address(filters[i], uuid.c_str());
        }

        if(!res){
            btlog::Logger::log(btlog::LogLevel::DEBUG, "starting scan...");
            res = bt_adapter_le_start_scan(&BluetoothManager::scanCallback, this);
        }
        if(res){
            std::string err=get_error_message(res);
            Logger::log(LogLevel::ERROR, "scanning start failed with " + err);
        }
        for(auto& f : filters){
            bt_adapter_le_scan_filter_destroy(&f);
        }
    }

    auto BluetoothManager::scanCallback(int result, bt_adapter_le_device_scan_result_info_s *discovery_info, void *user_data) noexcept -> void {
        BluetoothManager& bluetoothManager = *static_cast<BluetoothManager*> (user_data);
        using State=BluetoothDeviceController::State;
        if(result)
            Logger::log(LogLevel::ERROR, "NULLPTR IN CALL!");
        else{
            std::string macAddress=discovery_info->remote_address;
            std::scoped_lock lock(bluetoothManager._bluetoothDevices.mut, bluetoothManager._scanAllowDuplicates.mut);
            std::shared_ptr<BluetoothDeviceController> device;
            if(bluetoothManager._bluetoothDevices.var.find(macAddress)==bluetoothManager._bluetoothDevices.var.end())
                device=(*(bluetoothManager._bluetoothDevices.var.insert({macAddress, std::make_shared<BluetoothDeviceController>(macAddress, bluetoothManager._notificationsHandler)}).first)).second;
            else
                device=(*bluetoothManager._bluetoothDevices.var.find(macAddress)).second;
                
            if(bluetoothManager._scanAllowDuplicates.var || device->cProtoBluetoothDevices().empty()){
                device->protoBluetoothDevices().emplace_back();
                            
                auto& protoDev=device->protoBluetoothDevices().back();
                char* name;
                result=bt_adapter_le_get_scan_result_device_name(discovery_info, BT_ADAPTER_LE_PACKET_SCAN_RESPONSE, &name);
                if(!result){
                    protoDev.set_name(name);
                    free(name);
                }
                protoDev.set_remote_id(macAddress);

                proto::gen::ScanResult scanResult;
                scanResult.set_rssi(discovery_info->rssi);
                proto::gen::AdvertisementData* advertisement_data=new proto::gen::AdvertisementData();
                decodeAdvertisementData(discovery_info->adv_data, *advertisement_data, discovery_info->adv_data_len);

                scanResult.set_allocated_advertisement_data(advertisement_data);
                scanResult.set_allocated_device(new proto::gen::BluetoothDevice(protoDev));

                bluetoothManager._notificationsHandler.notifyUIThread("ScanResult", scanResult);
            }
        }
    }
    
    auto BluetoothManager::stopBluetoothDeviceScanLE() noexcept -> void{
        static std::mutex m;
        std::scoped_lock lock(m);
        auto btState=bluetoothState().state();
        if(btState==proto::gen::BluetoothState_State_ON){
            bool isDiscovering;
            int res = bt_adapter_le_is_discovering(&isDiscovering);
            if(!res && isDiscovering){
                res = bt_adapter_le_stop_scan();
                if(res){
                    std::string err=get_error_message(res);
                    Logger::log(LogLevel::ERROR, "disabling scan failed with " + err);
                    Logger::log(LogLevel::ERROR, "is Discovering=" + std::to_string(isDiscovering));
                }else
                    btlog::Logger::log(btlog::LogLevel::DEBUG, "scan stopped.");
            }else{
                btlog::Logger::log(btlog::LogLevel::WARNING, "Cannot stop scan. It is not in progress!");
            }
        }else{
            Logger::log(LogLevel::ERROR, "bluetooth adapter state="+std::to_string(btState));
        }
    }

    auto BluetoothManager::connect(const proto::gen::ConnectRequest& connRequest) noexcept -> void {
        std::unique_lock lock(_bluetoothDevices.mut);
        using State=BluetoothDeviceController::State;
        auto device=(*_bluetoothDevices.var.find(connRequest.remote_id())).second;
        device->connect(connRequest);
    }

    auto BluetoothManager::disconnect(const std::string& deviceID) noexcept -> void {
        std::unique_lock lock(_bluetoothDevices.mut);
        using State=BluetoothDeviceController::State;
        auto device=(*_bluetoothDevices.var.find(deviceID)).second;
        device->disconnect();
    }

    auto BluetoothManager::serviceSearch(const proto::gen::BluetoothDevice& bluetoothDevice) noexcept -> void {
        int res = bt_device_start_service_search(bluetoothDevice.remote_id().c_str());
        if(res){
            std::string err=get_error_message(res);
            Logger::log(LogLevel::ERROR, "device service search failed with " + err);
        }
    }

    auto BluetoothManager::getConnectedProtoBluetoothDevices() noexcept -> std::vector<proto::gen::BluetoothDevice> {
        std::vector<proto::gen::BluetoothDevice> protoBD;
        std::scoped_lock lock(_bluetoothDevices.mut);
        using State=BluetoothDeviceController::State;
        for(const auto& e:_bluetoothDevices.var){
            if(e.second->state()==State::CONNECTED){
                auto& vec=e.second->cProtoBluetoothDevices();
                protoBD.insert(protoBD.end(), vec.cbegin(), vec.cend());
            }
        }
        return protoBD;
    }

    auto BluetoothManager::bluetoothDevices() noexcept -> decltype(_bluetoothDevices)& { return _bluetoothDevices; }

    auto decodeAdvertisementData(char* packetsData, proto::gen::AdvertisementData& adv, int dataLen) -> void {
        using byte=char;
        int start=0;
        bool longNameSet=false;
        while(start<dataLen){
            byte ad_len=packetsData[start]&0xFFu;
            byte type=packetsData[start+1]&0xFFu;

            byte* packet=packetsData+start+2;
            switch(type){
                case 0x09:
                case 0x08:{
                    if(!longNameSet) {
                        adv.set_local_name(packet, ad_len-1);
                    }
                    
                    if(type==0x09) longNameSet=true;
                    break;
                }
                case 0x01:{
                    adv.set_connectable(*packet & 0x3);
                    break;
                }
                case 0xFF:{
                    break;
                }
                default: break;
            }
            start+=ad_len+1;
        }
    }

} // namespace btu

