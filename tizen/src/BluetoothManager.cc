#include <BluetoothManager.h>

#include <system_info.h>
#include <Logger.h>
#include <LogLevel.h>

#include <bluetooth.h>
#include <tizen.h>



namespace btu{
    AdvertisementData advertisementDataBuildFromRaw(char* dataRaw, int dataLength);
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
            AdvertisementData* advertisementData = new AdvertisementData(advertisementDataBuildFromRaw(discovery_info.adv_data, discovery_info.adv_data_len));
            char* name;
            int res = bt_adapter_le_get_scan_result_device_name(&discovery_info, BT_ADAPTER_LE_PACKET_SCAN_RESPONSE, &name);
            if(res){
                std::string err=get_error_message(res);
                Logger::log(LogLevel::ERROR, "fetching device name failed with " + err);
            }else{
                bluetoothDevice->set_name(name);
                free(name);
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
    
    AdvertisementData advertisementDataBuildFromRaw(char* dataRaw, int dataLength){ 
        //[TODO] DESERIALIZE
        return AdvertisementData();
    }

    void BluetoothManager::connect(const ConnectRequest& connRequest) noexcept{
        // bt_device_create_bond(connRequest.remote_id().c_str());
        //wait for completion

        Logger::log(LogLevel::DEBUG, "remote id is - " + connRequest.remote_id());

        // std::unique_lock lock(pendingConnectionRequests.mut);

        // auto& pending = pendingConnectionRequests.var[connRequest.remote_id()];
        // pending.first.wait(lock, [&]() -> bool{
        //    return pending.second;
        // });
    }
    void BluetoothManager::deviceConnectedCallback(int result, bt_device_info_s *device_info, void *user_data) noexcept{
        if(result){
            Logger::log(LogLevel::ERROR, "FAILED TO CONNECT!");
            return;
        }
        BluetoothManager& bluetoothManager = *static_cast<BluetoothManager*> (user_data);
        std::scoped_lock lock(bluetoothManager.pendingConnectionRequests.mut);
        // bluetoothManager.pendingConnectionRequests.var[device_info->bt_class.]

        // std::scoped_lock lock(mapp.mut);
        // mapp.var.insert({})
    }
} // namespace btu

