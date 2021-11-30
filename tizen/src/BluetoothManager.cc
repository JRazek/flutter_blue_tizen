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
        if(!getBluetoothAvailability()){
            Logger::log(LogLevel::ERROR, "Bluetooth is not available on this device!");
            return;
        }
        if(bt_initialize() != BT_ERROR_NONE){
            Logger::log(LogLevel::ERROR, "[bt_initialize] failed");
            return;
        }

        int res = bt_adapter_set_state_changed_cb(BluetoothManager::adapterStateChangedCallback, this);
        if(res != BT_ERROR_NONE){
            Logger::log(LogLevel::ERROR, "could not set adapter state callback function! err_code: " + std::to_string(res));
            return;
        }
        res = bt_adapter_set_device_discovery_state_changed_cb(BluetoothManager::adapterDeviceDiscoveryStateChangedCallback, this);
        if(res != BT_ERROR_NONE){
            Logger::log(LogLevel::ERROR, "could not set discovery callback! err_code: " + std::to_string(res));
            return;
        }
        Logger::log(LogLevel::DEBUG, "All callbacks successfully initialized.");
    }

    BluetoothManager::~BluetoothManager() noexcept{
        if(bt_deinitialize() != BT_ERROR_NONE){
            Logger::log(LogLevel::ERROR, "[bt_deinitialize] failed");
        }
    }

    bool BluetoothManager::getBluetoothAvailability() noexcept{
        bool state;
        int ret = system_info_get_platform_bool("http://tizen.org/feature/network.bluetooth", &state);
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

    void BluetoothManager::startBluetoothDeviceDiscovery() const noexcept{
        bool state;
        int res = bt_adapter_is_discovering(&state);
        if(res != BT_ERROR_NONE){
            Logger::log(LogLevel::ERROR, "error during running scan!");
        }
        if(!state){
            res = bt_adapter_start_device_discovery();
            if(res == BT_ERROR_NOW_IN_PROGRESS){
                Logger::log(LogLevel::DEBUG, "scan already running. skipping.");
            }else if(res != BT_ERROR_NONE){
                Logger::log(LogLevel::ERROR, "error during running scan!");
            }
        }
    }   

    void BluetoothManager::stopBluetoothDeviceDiscovery() const noexcept{
        int res = bt_adapter_stop_device_discovery();
        if(res != BT_ERROR_NONE){
            if(res == BT_ERROR_NOT_INITIALIZED)
                Logger::log(LogLevel::ERROR, "error in disabling scan. BT_ERROR_NOT_INITIALIZED" );
            else 
                Logger::log(LogLevel::ERROR, "error in disabling scan. " + std::to_string(res));
        }
            

        Logger::log(LogLevel::DEBUG, "stopping scan...");
    }

    void BluetoothManager::adapterDeviceDiscoveryStateChangedCallback(int result, bt_adapter_device_discovery_state_e discovery_state, bt_adapter_device_discovery_info_s *discovery_info, void* user_data) noexcept{
        BluetoothManager& bluetoothManager = *static_cast<BluetoothManager*> (user_data);
        Logger::log(LogLevel::DEBUG, "Callback im here :DD.");

        if(discovery_state == BT_ADAPTER_DEVICE_DISCOVERY_FOUND){
            bluetoothManager.addDiscoveryResult(*discovery_info);
            Logger::log(LogLevel::DEBUG, "FOUND A NEW DEVICE");
        }else if(discovery_state == BT_ADAPTER_DEVICE_DISCOVERY_FINISHED){
            Logger::log(LogLevel::DEBUG, "BT_ADAPTER_DEVICE_DISCOVERY_FINISHED");
        }else{
            Logger::log(LogLevel::DEBUG, "BT_ADAPTER_DEVICE_DISCOVERY_STARTED");
        }
    }
    void BluetoothManager::addDiscoveryResult(bt_adapter_device_discovery_info_s& discovery_info) noexcept{        
        //[TODO] TEST THIS FUNCTION
        ScanResult scanResult;
        BluetoothDevice bluetoothDevice;

        bluetoothDevice.set_remote_id(discovery_info.remote_address);
        bluetoothDevice.set_name(discovery_info.remote_name);
        bluetoothDevice.set_type(BluetoothDevice_Type_UNKNOWN);//[TODO] ??

        scanResult.set_allocated_device(&bluetoothDevice);
        scanResult.set_rssi(discovery_info.rssi);

        std::vector<uint8_t> encodable(scanResult.ByteSizeLong());
        scanResult.SerializeToArray(encodable.data(), scanResult.ByteSizeLong());
        methodChannel->InvokeMethod("ScanResult", std::make_unique<flutter::EncodableValue>(encodable));
        Logger::log(LogLevel::DEBUG, "sent new scan result to flutter.");
    }

    SafeType<std::vector<BluetoothDevice>>& BluetoothManager::getConnectedDevices() noexcept{
        return connectedDevices;
    }

} // namespace btu

