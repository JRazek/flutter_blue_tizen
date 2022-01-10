#include <BluetoothManager.h>

#include <system_info.h>
#include <Logger.h>
#include <LogLevel.h>

#include <bluetooth.h>
#include <tizen.h>



namespace btu{
    using LogLevel = btlog::LogLevel;
    using Logger = btlog::Logger;
    auto decodeAdvertisementData(char* packetsData, AdvertisementData& adv, int dataLen) -> void;


    BluetoothManager::BluetoothManager(std::shared_ptr<MethodChannel> _methodChannel) noexcept:
    methodChannel(_methodChannel){
        if(isBLEAvailable()){
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
        if(bt_device_set_service_searched_cb(&BluetoothManager::serviceSearchCallback, this)){
            Logger::log(LogLevel::ERROR, "[bt_device_set_service_searched_cb] failed");
            return;
        }
        Logger::log(LogLevel::DEBUG, "All callbacks successfully initialized.");
    }

    BluetoothManager::~BluetoothManager() noexcept{
        if(bt_deinitialize() != BT_ERROR_NONE){
            Logger::log(LogLevel::ERROR, "[bt_deinitialize] failed");
        }
    }

    auto BluetoothManager::isBLEAvailable() noexcept -> bool{
        bool state;
        int ret = system_info_get_platform_bool("http://tizen.org/feature/network.bluetooth.le", &state);
        if(ret != SYSTEM_INFO_ERROR_NONE){
            Logger::log(LogLevel::ERROR, "failed fetching bluetooth data!");
        }
        return state;
    }

    auto BluetoothManager::bluetoothState() const noexcept -> BluetoothState{
        /* Checks whether the Bluetooth adapter is enabled */
        bt_adapter_state_e adapter_state;
        int ret = bt_adapter_get_state(&adapter_state);
        BluetoothState bts;
        if(ret == BT_ERROR_NONE){
            if(adapter_state == BT_ADAPTER_ENABLED)
                bts.set_state(BluetoothState_State_ON);
            else
                bts.set_state(BluetoothState_State_OFF);
        }
        else if(ret == BT_ERROR_NOT_INITIALIZED)
            bts.set_state(BluetoothState_State_UNAVAILABLE);
        else
            bts.set_state(BluetoothState_State_UNKNOWN);
        
        return bts;
    }

    auto BluetoothManager::adapterStateChangedCallback(int result, bt_adapter_state_e adapter_state, void* user_data) noexcept -> void {
        BluetoothManager& bluetoothManager = *static_cast<BluetoothManager*> (user_data);
        std::scoped_lock lock(bluetoothManager.adapterState.mut);
        bluetoothManager.adapterState.var = adapter_state;
    }
    
    auto BluetoothManager::startBluetoothDeviceScanLE(const ScanSettings& scanSettings) noexcept -> void {
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
            auto& device=bluetoothManager._bluetoothDevices.var[macAddress];
            device.address()=macAddress;
            device.state()=State::SCANNED;
            if(bluetoothManager._scanAllowDuplicates.var || device.cProtoBluetoothDevices().empty()){
                device.protoBluetoothDevices().emplace_back();
                            
                auto& protoDev=device.protoBluetoothDevices().back();
                char* name;
                result=bt_adapter_le_get_scan_result_device_name(discovery_info, BT_ADAPTER_LE_PACKET_SCAN_RESPONSE, &name);
                if(!result){
                    protoDev.set_name(name);
                    free(name);
                }
                protoDev.set_remote_id(macAddress);

                ScanResult scanResult;
                scanResult.set_rssi(discovery_info->rssi);
                AdvertisementData* advertisement_data=new AdvertisementData();
                decodeAdvertisementData(discovery_info->adv_data, *advertisement_data, discovery_info->adv_data_len);
                // Logger::log(LogLevel::WARNING, std::to_string(discovery_info->adv_data_len));
                //decode advertisment data here...[TODO]
                scanResult.set_allocated_advertisement_data(advertisement_data);
                scanResult.set_allocated_device(new BluetoothDevice(protoDev));

                std::vector<uint8_t> encodable(scanResult.ByteSizeLong());
                scanResult.SerializeToArray(encodable.data(), scanResult.ByteSizeLong());
                bluetoothManager.methodChannel->InvokeMethod("ScanResult", std::make_unique<flutter::EncodableValue>(encodable));
            }
        }
    }
    
    auto BluetoothManager::stopBluetoothDeviceScanLE() noexcept -> void{
        static std::mutex m;
        std::scoped_lock lock(m);
        auto btState=bluetoothState().state();
        if(btState==BluetoothState_State_ON){
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

    auto BluetoothManager::connect(const ConnectRequest& connRequest) noexcept -> void {
        std::unique_lock lock(_bluetoothDevices.mut);
        using State=BluetoothDeviceController::State;
        auto& device=_bluetoothDevices.var[connRequest.remote_id()];
        device.connect(connRequest);
    }

    auto BluetoothManager::disconnect(const std::string& deviceID) noexcept -> void {
        std::unique_lock lock(_bluetoothDevices.mut);
        using State=BluetoothDeviceController::State;
        auto& device=_bluetoothDevices.var[deviceID];
        device.disconnect();
    }

    auto BluetoothManager::serviceSearch(const BluetoothDevice& bluetoothDevice) noexcept -> void {
        int res = bt_device_start_service_search(bluetoothDevice.remote_id().c_str());
        if(res){
            std::string err=get_error_message(res);
            Logger::log(LogLevel::ERROR, "device service search failed with " + err);
        }
    }

    auto BluetoothManager::serviceSearchCallback(int result, bt_device_sdp_info_s* services, void* user_data) noexcept -> void {
        BluetoothManager& bluetoothManager=*static_cast<BluetoothManager *>(user_data);
        Logger::log(LogLevel::DEBUG, "discovered services for " + std::string(services->remote_address));

        for(int i=0;i<services->service_count;i++){

        }
    }

    auto BluetoothManager::getConnectedProtoBluetoothDevices() noexcept -> std::vector<BluetoothDevice> {
        std::vector<BluetoothDevice> protoBD;
        std::scoped_lock lock(_bluetoothDevices.mut);
        using State=BluetoothDeviceController::State;
        for(const auto& e:_bluetoothDevices.var){
            if(e.second.cState()==State::CONNECTED){
                auto& vec=e.second.cProtoBluetoothDevices();
                protoBD.insert(protoBD.end(), vec.cbegin(), vec.cend());
            }
        }
        return protoBD;
    }

    auto BluetoothManager::bluetoothDevices() noexcept -> decltype(_bluetoothDevices)& { return _bluetoothDevices; }
    
    /*
    0x01 = flags
    0x03 = Complete List of 16-bit Service Class UUIDs
    0x09 = Complete Local Name
    0x08 = Shortened Local Name
    */
    auto decodeAdvertisementData(char* packetsData, AdvertisementData& adv, int dataLen) -> void {
        using byte=u_int8_t;
        int start=0;
        bool longNameSet=false;
        while(start<dataLen){
            byte ad_len=packetsData[start]&0xFFu;
            byte type=packetsData[start+1]&0xFFu;
            switch(type){
                case 0x09:
                case 0x08:{
                    if(!longNameSet) {
                        adv.set_local_name(packetsData+start+2, ad_len-1);
                        // Logger::log(LogLevel::DEBUG, "local name was set");
                    }
                    
                    if(type==0x09) longNameSet=true;
                    break;
                }
                case 0xFF:{
                    break;
                }
                default: break;
            }
            // Logger::log(LogLevel::DEBUG, "found type of packet:len="+std::to_string(type)+":"+std::to_string(ad_len));
            // Logger::log(LogLevel::DEBUG, "dataLen="+std::to_string(dataLen));
            start+=ad_len+1;
        }
        // std::string tmps="";
        // for(int i=0;i<dataLen;i++){
        //     tmps+=std::to_string(packetsData[i])+", ";
        // }
        // Logger::log(LogLevel::DEBUG, tmps);
    }

} // namespace btu

