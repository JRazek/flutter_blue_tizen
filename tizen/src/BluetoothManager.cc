#include <BluetoothManager.h>

#include <system_info.h>
#include <Logger.h>
#include <LogLevel.h>

#include <bluetooth.h>


#include <flutterblue.pb.h>

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
            discoveredDevicesAddresses.clear();
            res = bt_adapter_le_start_scan(&BluetoothManager::adapterDeviceDiscoveryStateChangedCallbackLE, this);
        }else
            Logger::log(LogLevel::ERROR, "scanning start failed with " + std::to_string(res));
               
    }   

    void BluetoothManager::stopBluetoothDeviceScanLE() noexcept{
        bool isDiscovering;
        int res = bt_adapter_le_is_discovering(&isDiscovering);
        if(isDiscovering){
            res = bt_adapter_le_stop_scan();
            if(res != BT_ERROR_NONE){
                if(res == BT_ERROR_NOT_INITIALIZED)
                    Logger::log(LogLevel::ERROR, "error in disabling scan. BT_ERROR_NOT_INITIALIZED");
                else if(res == BT_ERROR_NOT_IN_PROGRESS)
                    Logger::log(LogLevel::ERROR, "error in disabling scan. BT_ERROR_NOT_IN_PROGRESS");
                else 
                    Logger::log(LogLevel::ERROR, "error in disabling scan. " + std::to_string(res));
            }
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
        if(discoveredDevicesAddresses.find(address) == discoveredDevicesAddresses.end()){
            ScanResult scanResult;
            BluetoothDevice* bluetoothDevice = new BluetoothDevice();
            AdvertisementData* advertisementData = new AdvertisementData();
            bluetoothDevice->set_allocated_name(new std::string(address));
            
            scanResult.set_allocated_advertisement_data(advertisementData);
            scanResult.set_allocated_device(bluetoothDevice);
            std::vector<uint8_t> encodable(scanResult.ByteSizeLong());
            scanResult.SerializeToArray(encodable.data(), scanResult.ByteSizeLong());
            discoveredDevicesAddresses.insert(address);
            methodChannel->InvokeMethod("ScanResult", std::make_unique<flutter::EncodableValue>(encodable));
            Logger::log(LogLevel::DEBUG, "sent new scan result to flutter. " + address);
        }
    }

    SafeType<std::vector<BluetoothDevice>>& BluetoothManager::getConnectedDevicesLE() noexcept{
        return connectedDevices;
    }
    
    AdvertisementData advertisementDataBuildFromRaw(char* dataRaw){
        
    }
} // namespace btu

