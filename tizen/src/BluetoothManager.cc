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
        if(bt_initialize() != BT_ERROR_NONE){
            Logger::log(LogLevel::ERROR, "[bt_initialize] failed");
            return;
        }
        if(bt_device_set_bond_created_cb(&BluetoothManager::deviceConnectedCallback, this) != BT_ERROR_NONE){
            Logger::log(LogLevel::ERROR, "[bt_device_set_bond_created_cb] failed");
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
        /* Check whether the Bluetooth adapter is enabled */
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
        // int res =  bt_adapter_le_set_scan_mode()
        bool isDiscovering;  
        int res = bt_adapter_le_is_discovering(&isDiscovering);
        if(isDiscovering)
            stopBluetoothDeviceScanLE();
        if(!res){
            std::scoped_lock lock(discoveredDevicesAddresses.mut);
            discoveredDevicesAddresses.var.clear();
            btlog::Logger::log(btlog::LogLevel::DEBUG, "starting scan...");
            res = bt_adapter_le_start_scan(&BluetoothManager::adapterDeviceDiscoveryStateChangedCallbackLE, this);
        }
        if(res){
            std::string err=get_error_message(res);
            Logger::log(LogLevel::ERROR, "scanning start failed with " + err);
        }
        
    }   

    void BluetoothManager::stopBluetoothDeviceScanLE() noexcept{
        bool isDiscovering;
        int res = bt_adapter_le_is_discovering(&isDiscovering);
        if(isDiscovering){
            res = bt_adapter_le_stop_scan();
            if(res){
                std::string err=get_error_message(res);
                Logger::log(LogLevel::ERROR, "disabling scan failed with " + err);
            }else
                btlog::Logger::log(btlog::LogLevel::DEBUG, "scan stopped.");
        }else{
            btlog::Logger::log(btlog::LogLevel::WARNING, "Cannot stop scan. It is not in progress!");
        }
    }

    void BluetoothManager::adapterDeviceDiscoveryStateChangedCallbackLE(int result, bt_adapter_le_device_scan_result_info_s *discovery_info, void *user_data) {
        BluetoothManager& bluetoothManager = *static_cast<BluetoothManager*> (user_data);
        if(result)
            Logger::log(LogLevel::ERROR, "NULLPTR IN CALL!");
        else{
            bluetoothManager.notifyDiscoveryResultLE(*discovery_info);
        }
    }

    void BluetoothManager::notifyDiscoveryResultLE(const bt_adapter_le_device_scan_result_info_s& discovery_info){        
        std::string address(discovery_info.remote_address);
        std::scoped_lock lock(discoveredDevicesAddresses.mut);
        if(discoveredDevicesAddresses.var.find(address) == discoveredDevicesAddresses.var.end()){
            ScanResult scanResult;
            BluetoothDevice* bluetoothDevice = new BluetoothDevice();
            AdvertisementData* advertisementData = new AdvertisementData();

            char* name;
            int res = bt_adapter_le_get_scan_result_device_name(&discovery_info, BT_ADAPTER_LE_PACKET_ADVERTISING, &name);
            if(res){
                std::string err=get_error_message(res);
                Logger::log(LogLevel::ERROR, "fetching device name failed with " + err);
            }else{
                bluetoothDevice->set_name(name);
                free(name);
            }

            char** uuids;
            int uuidsCount;
            res = bt_adapter_le_get_scan_result_service_uuids(&discovery_info, BT_ADAPTER_LE_PACKET_SCAN_RESPONSE, &uuids, &uuidsCount);
            if(res){
                std::string err=get_error_message(res);
                Logger::log(LogLevel::ERROR, "fetching device uuids failed with " + err);
            }else{
                bluetoothDevice->set_name(name);
                for(int i=0;i<uuidsCount;i++){
                    std::string uuid(uuids[i]);
                    free(uuids[i]);
                    Logger::log(LogLevel::DEBUG, "fetched scan service uuid " + uuid);
                }
                free(uuids);
            }
            // bluetoothDevice->set_name(address);
            bluetoothDevice->set_remote_id(address);
            
            scanResult.set_allocated_advertisement_data(advertisementData);
            scanResult.set_allocated_device(bluetoothDevice);
            scanResult.set_rssi(discovery_info.rssi);
            std::vector<uint8_t> encodable(scanResult.ByteSizeLong());
            scanResult.SerializeToArray(encodable.data(), scanResult.ByteSizeLong());

            discoveredDevicesAddresses.var.insert({address, bluetoothDevice});//pointer instead!!

            methodChannel->InvokeMethod("ScanResult", std::make_unique<flutter::EncodableValue>(encodable));
            Logger::log(LogLevel::DEBUG, "sent new scan result to flutter. " + address);
        }
    }

    SafeType<std::unordered_map<std::string, BluetoothDevice>>& BluetoothManager::getConnectedDevicesLE() noexcept{
        return connectedDevices;
    }

    void BluetoothManager::connect(const ConnectRequest& connRequest) noexcept{
        using namespace std::literals;
        bt_device_create_bond(connRequest.remote_id().c_str());

        Logger::log(LogLevel::DEBUG, "remote id is - " + connRequest.remote_id());

        std::unique_lock lock(pendingConnectionRequests.mut);

        //wait for completion
        auto& pending = pendingConnectionRequests.var[connRequest.remote_id()];
        auto timeout=std::chrono::steady_clock::now()+2s;
        pending.first.wait(lock, [&]() -> bool{
           return pending.second;
        });

        if(!pending.second){
            pendingConnectionRequests.var.erase(connRequest.remote_id());
        }    
        Logger::log(LogLevel::DEBUG, "connection request released. Device found status="+std::to_string(pending.second));
    }
    void BluetoothManager::deviceConnectedCallback(int result, bt_device_info_s* device_info, void* user_data) noexcept{
        Logger::log(LogLevel::DEBUG, "callback with remote_address="+std::string(device_info->remote_address));
        if(result){
            Logger::log(LogLevel::ERROR, "FAILED TO CONNECT!");
        }
        BluetoothManager& bluetoothManager = *static_cast<BluetoothManager*> (user_data);
        std::scoped_lock lock(bluetoothManager.pendingConnectionRequests.mut);
        
        Logger::log(LogLevel::DEBUG, "connected to device - "+std::string(device_info->remote_address));
        //remote_id==remote_address
        auto& mapp=bluetoothManager.pendingConnectionRequests.var;
        if(mapp.find(device_info->remote_address)!=mapp.end()){
            auto& p=mapp[device_info->remote_address];
            p.second=true;
            p.first.notify_one();
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
    
    #ifndef NDEBUG
    void BluetoothManager::testConnect(){
        static std::once_flag flag;
        // const char* remoteAddress="D0:1B:49:81:35:A7";
        std::call_once(flag, [&](){
            ConnectRequest request;
            std::string remoteID;
            {
                std::scoped_lock lock(discoveredDevicesAddresses.mut);
                remoteID=(*discoveredDevicesAddresses.var.begin()).first;
            }
            request.set_remote_id(remoteID);

            // request.set_remote_id(remoteAddress);
            connect(request);
        });
    }
    #endif
} // namespace btu

