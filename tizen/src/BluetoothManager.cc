#include <BluetoothManager.h>

#include <system_info.h>
#include <Logger.h>
#include <LogLevel.h>

#include <bluetooth.h>
#include <tizen.h>



namespace btu{
    using LogLevel = btlog::LogLevel;
    using Logger = btlog::Logger;

    BluetoothManager::BluetoothManager(std::shared_ptr<MethodChannel> _methodChannel) noexcept:
    methodChannel(_methodChannel){
        if(!getBluetoothAvailabilityLE()){
            Logger::log(LogLevel::ERROR, "Bluetooth is not available on this device!");
            return;
        }
        if(!bt_initialize()){
            Logger::log(LogLevel::ERROR, "[bt_initialize] failed");
            return;
        }
        if(!bt_device_set_bond_created_cb(&BluetoothManager::deviceConnectedCallback, this)){
            Logger::log(LogLevel::ERROR, "[bt_device_set_bond_created_cb] failed");
            return;
        }
        if(!bt_device_set_service_searched_cb(&BluetoothManager::serviceSearchCallback, this)){
            Logger::log(LogLevel::ERROR, "[bt_device_set_service_searched_cb] failed");
            return;
        }
        if(!bt_device_set_bond_destroyed_cb(&BluetoothManager::deviceDisconnectedCallback, this)){
            Logger::log(LogLevel::ERROR, "[bt_device_set_bond_destroyed_cb] failed");
            return;
        }
        Logger::log(LogLevel::DEBUG, "All callbacks successfully initialized.");
    }

    BluetoothManager::~BluetoothManager() noexcept{
        if(bt_deinitialize() != BT_ERROR_NONE){
            Logger::log(LogLevel::ERROR, "[bt_deinitialize] failed");
        }
    }

    bool BluetoothManager::getBluetoothAvailabilityLE() noexcept{
        bool state;
        int ret = system_info_get_platform_bool("http://tizen.org/feature/network.bluetooth.le", &state);
        if(ret != SYSTEM_INFO_ERROR_NONE){
            Logger::log(LogLevel::ERROR, "failed fetching bluetooth data!");
        }
        return state;
    }

    BluetoothState BluetoothManager::getBluetoothState() const noexcept{
        /* Checks whether the Bluetooth adapter is enabled */
        bt_adapter_state_e adapter_state;
        int ret = bt_adapter_get_state(&adapter_state);
        BluetoothState bluetoothState;
        if(ret == BT_ERROR_NONE){
            if(adapter_state == BT_ADAPTER_ENABLED)
                bluetoothState.set_state(BluetoothState_State_ON);
            else
                bluetoothState.set_state(BluetoothState_State_OFF);
        }
        else if(ret == BT_ERROR_NOT_INITIALIZED)
            bluetoothState.set_state(BluetoothState_State_UNAVAILABLE);
        else
            bluetoothState.set_state(BluetoothState_State_UNKNOWN);
        
        return bluetoothState;
    }

    void BluetoothManager::adapterStateChangedCallback(int result, bt_adapter_state_e adapter_state, void* user_data) noexcept{
        BluetoothManager& bluetoothManager = *static_cast<BluetoothManager*> (user_data);
        bluetoothManager.setAdapterState(adapter_state);
    }
    void BluetoothManager::setAdapterState(bt_adapter_state_e _state) noexcept{
        std::scoped_lock lock(adapterState.mut);
        adapterState.var = _state;
    }
    
    void BluetoothManager::startBluetoothDeviceScanLE(const ScanSettings& scanSettings) noexcept{
        std::scoped_lock l(bluetoothDevices.mut, scanAllowDuplicates.mut);
        bluetoothDevices.var.clear();        
        scanAllowDuplicates.var=scanSettings.allow_duplicates();        
        Logger::log(LogLevel::DEBUG, "allowDuplicates="+std::to_string(scanAllowDuplicates.var));

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

    void BluetoothManager::scanCallback(int result, bt_adapter_le_device_scan_result_info_s *discovery_info, void *user_data) {
        BluetoothManager& bluetoothManager = *static_cast<BluetoothManager*> (user_data);
        using State=BluetoothDeviceController::State;
        if(result)
            Logger::log(LogLevel::ERROR, "NULLPTR IN CALL!");
        else{
            std::string macAddress=discovery_info->remote_address;
            std::scoped_lock lock(bluetoothManager.bluetoothDevices.mut, bluetoothManager.scanAllowDuplicates.mut);
            auto& device=bluetoothManager.bluetoothDevices.var[macAddress];
            device.address()=macAddress;
            device.state()=State::SCANNED;
            if(bluetoothManager.scanAllowDuplicates.var || device.cProtoBluetoothDevices().empty()){
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
                //decode advertisment data here...[TODO]
                scanResult.set_allocated_advertisement_data(advertisement_data);
                scanResult.set_allocated_device(new BluetoothDevice(protoDev));

                std::vector<uint8_t> encodable(scanResult.ByteSizeLong());
                scanResult.SerializeToArray(encodable.data(), scanResult.ByteSizeLong());
                bluetoothManager.methodChannel->InvokeMethod("ScanResult", std::make_unique<flutter::EncodableValue>(encodable));
            }
        }
    }

    void BluetoothManager::stopBluetoothDeviceScanLE() noexcept{
        static std::mutex m;
        std::scoped_lock lock(m);
        auto btState=getBluetoothState().state();
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

    void BluetoothManager::connect(const ConnectRequest& connRequest) noexcept{
        std::unique_lock lock(bluetoothDevices.mut);
        using State=BluetoothDeviceController::State;
        auto& device=bluetoothDevices.var[connRequest.remote_id()];
        device.connect(connRequest);
    }
    void BluetoothManager::deviceConnectedCallback(int result, bt_device_info_s* device_info, void* user_data) noexcept{
        Logger::log(LogLevel::DEBUG, "callback with remote_address="+std::string(device_info->remote_address));
        using State=BluetoothDeviceController::State;
        
    }

    void BluetoothManager::disconnect(const std::string& deviceID) noexcept{
        using namespace std::literals;
        std::unique_lock lock(bluetoothDevices.mut);
        auto& device=bluetoothDevices.var[deviceID];
        using State=BluetoothDeviceController::State;
        auto timeout=std::chrono::steady_clock::now()+5s;
 
    }
    void BluetoothManager::deviceDisconnectedCallback(int res, char* remote_address, void* user_data) noexcept{
        if(!res){
            BluetoothManager& bluetoothManager = *static_cast<BluetoothManager*> (user_data);

        }else{
            std::string err=get_error_message(res);
            Logger::log(LogLevel::ERROR, "crating advertiser failed with " + err);
        }
    }

    void BluetoothManager::startAdvertising() noexcept{
        std::scoped_lock lock(advertisingHandler.mut);
        auto& handle=advertisingHandler.var;
        int res=bt_adapter_le_create_advertiser(&handle);//[TODO - destroy the handle!]
        if(res){
            std::string err=get_error_message(res);
            Logger::log(LogLevel::ERROR, "crating advertiser failed with " + err);
        }
        res=bt_adapter_le_add_advertising_service_uuid(handle, BT_ADAPTER_LE_PACKET_ADVERTISING, "3bf5fc18-6c3c-11ec-90d6-0242ac120003");
        if(res){
            std::string err=get_error_message(res);
            Logger::log(LogLevel::ERROR, "setting advertising uuids failed with " + err);
        }
        res=bt_adapter_le_start_advertising_new(handle, &BluetoothManager::adapterAdvertisingStateChangedCallbackLE, this);
        if(res){
            std::string err=get_error_message(res);
            Logger::log(LogLevel::ERROR, "starting advertising failed with " + err);
        }
    }

    void BluetoothManager::adapterAdvertisingStateChangedCallbackLE(int result, bt_advertiser_h advertiser, bt_adapter_le_advertising_state_e adv_state, void *user_data){
        BluetoothManager& bluetoothManager=*static_cast<BluetoothManager *>(user_data);
        std::scoped_lock lock(bluetoothManager.advertisingHandler.mut);
        auto& handle=bluetoothManager.advertisingHandler.var;
        Logger::log(LogLevel::DEBUG, "state of advertising handler changed to " + std::string(adv_state==BT_ADAPTER_LE_ADVERTISING_STARTED ? "STARTED" : "STOPPED"));

    }
    
    void BluetoothManager::serviceSearch(const BluetoothDevice& bluetoothDevice) noexcept{
        int res = bt_device_start_service_search(bluetoothDevice.remote_id().c_str());
        if(res){
            std::string err=get_error_message(res);
            Logger::log(LogLevel::ERROR, "device service search failed with " + err);
        }
    }

    void BluetoothManager::serviceSearchCallback(int result, bt_device_sdp_info_s* services, void* user_data) noexcept{
        BluetoothManager& bluetoothManager=*static_cast<BluetoothManager *>(user_data);
        Logger::log(LogLevel::DEBUG, "discovered services for " + std::string(services->remote_address));

        for(int i=0;i<services->service_count;i++){

        }
    }

    std::vector<BluetoothDevice> BluetoothManager::getConnectedProtoBluetoothDevices() noexcept{
        std::vector<BluetoothDevice> protoBD;
        std::scoped_lock lock(bluetoothDevices.mut);
        using State=BluetoothDeviceController::State;
        for(const auto& e:bluetoothDevices.var){
            if(e.second.cState()==State::CONNECTED){
                auto& vec=e.second.cProtoBluetoothDevices();
                protoBD.insert(protoBD.end(), vec.cbegin(), vec.cend());
            }
        }
        return protoBD;
    }
} // namespace btu

