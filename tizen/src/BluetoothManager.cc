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
        int res = bt_adapter_le_set_scan_mode(BT_ADAPTER_LE_SCAN_MODE_BALANCED);
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
            int res = bt_adapter_le_get_scan_result_device_name(&discovery_info, BT_ADAPTER_LE_PACKET_SCAN_RESPONSE, &name);
            if(!res){
                bluetoothDevice->set_name(name);
                free(name);
            }

            char** uuids;
            int uuidsCount;
            res = bt_adapter_le_get_scan_result_service_uuids(&discovery_info, BT_ADAPTER_LE_PACKET_SCAN_RESPONSE, &uuids, &uuidsCount);
            if(!res){
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

            discoveredDevicesAddresses.var.insert({address, bluetoothDevice});

            methodChannel->InvokeMethod("ScanResult", std::make_unique<flutter::EncodableValue>(encodable));
            Logger::log(LogLevel::DEBUG, "sent new scan result to flutter. " + address);
        }
    }

    SafeType<std::unordered_map<std::string, BluetoothDevice>>& BluetoothManager::getConnectedDevicesLE() noexcept{
        return connectedDevices;
    }

    void BluetoothManager::connect(const ConnectRequest& connRequest) noexcept{
        std::unique_lock lockConn(connectedDevices.mut);
        if(connectedDevices.var.find(connRequest.remote_id())==connectedDevices.var.end()){
            using namespace std::literals;
            bt_device_create_bond(connRequest.remote_id().c_str());

            Logger::log(LogLevel::DEBUG, "remote id is - " + connRequest.remote_id());
            
            std::unique_lock lock(pendingConnectRequests.mut);
            //wait for completion
            
            auto& pending = pendingConnectRequests.var[connRequest.remote_id()];
            auto timeout=std::chrono::steady_clock::now()+5s;
            pending.first.wait_until(lock, timeout, [&]() -> bool{
                return pending.second;
            });

            if(!pending.second)
                Logger::log(LogLevel::WARNING, "failed on connecting to device: "+connRequest.remote_id());
            else
                connectedDevices.var[connRequest.remote_id()];

            pendingConnectRequests.var.erase(connRequest.remote_id());
            Logger::log(LogLevel::DEBUG, "connection request released. Device found status="+std::to_string(pending.second));
        }else{
            Logger::log(LogLevel::WARNING, "requested connection to already connected device! - "+connRequest.remote_id());
        }
    }
    void BluetoothManager::deviceConnectedCallback(int result, bt_device_info_s* device_info, void* user_data) noexcept{
        Logger::log(LogLevel::DEBUG, "callback with remote_address="+std::string(device_info->remote_address));
        if(result==BT_ERROR_NONE||result==BT_ERROR_CANCELLED){
            BluetoothManager& bluetoothManager = *static_cast<BluetoothManager*> (user_data);
            std::scoped_lock lock(bluetoothManager.pendingConnectRequests.mut);
            
            Logger::log(LogLevel::DEBUG, "connected to device - "+std::string(device_info->remote_address));
            //remote_id==remote_address
            auto& mapp=bluetoothManager.pendingConnectRequests.var;
            if(mapp.find(device_info->remote_address)!=mapp.end()){
                auto& p=mapp[device_info->remote_address];
                if(result==BT_ERROR_NONE)
                    p.second=true;
                p.first.notify_one();
            }
        }else{
            Logger::log(LogLevel::ERROR, "FAILED TO CONNECT!");
        }
    }

    void BluetoothManager::disconnect(const std::string& deviceID) noexcept{
        using namespace std::literals;
        std::scoped_lock lock(pendingConnectRequests.mut, connectedDevices.mut);
        if(connectedDevices.var.find(deviceID)!=connectedDevices.var.end()){
            auto& pending=pendingConnectRequests.var[deviceID];
            pending.second=false;
            pending.first.notify_one();
        }else {
            std::unique_lock lock2(pendingDisconnectRequests.mut);
            if(pendingDisconnectRequests.var.find(deviceID)!=pendingDisconnectRequests.var.end()){
                bt_device_destroy_bond(deviceID.c_str());
                auto& pending=pendingDisconnectRequests.var[deviceID];
                auto timeout=std::chrono::steady_clock::now()+5s;
                pending.first.wait_until(lock2, timeout, [&]() -> bool{
                    return pending.second;
                });
                if(pending.second){
                    connectedDevices.var.erase(deviceID);
                }
            }
        }
    }
    void BluetoothManager::deviceDisconnectedCallback(int res, char* remote_address, void* user_data) noexcept{
        if(!res){
            BluetoothManager& bluetoothManager = *static_cast<BluetoothManager*> (user_data);
            auto& pending=bluetoothManager.pendingDisconnectRequests;
            std::scoped_lock lock(pending.mut);
            if(pending.var.find(remote_address)!=pending.var.end()){
                auto& p=pending.var[remote_address];
                p.second=true;
                p.first.notify_one();
            }
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

    #ifndef NDEBUG
    void BluetoothManager::testConnect(){
        static std::once_flag flag;
        std::call_once(flag, [&](){
            btlog::Logger::log(btlog::LogLevel::DEBUG, "test connect");
            ConnectRequest request;
            std::string remoteID;
            {
                std::scoped_lock lock(discoveredDevicesAddresses.mut);
                remoteID=(*discoveredDevicesAddresses.var.begin()).first;
            }
            request.set_remote_id(remoteID);
            connect(request);
        });
    }
    #endif
    
} // namespace btu

