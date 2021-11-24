#include <BluetoothManager.h>

#include <system_info.h>
#include <Logger.h>
#include <LogLevel.h>
#include <bluetooth.h>

#include <flutterblue.pb.h>

namespace btu{
    using LogLevel = btlog::LogLevel;
    using Logger = btlog::Logger;

    BluetoothManager::BluetoothManager() noexcept{
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
        bt_adapter_start_device_discovery();
    }

    void BluetoothManager::stopBluetoothDeviceDiscovery() const noexcept{
        bt_adapter_stop_device_discovery();
    }

    void BluetoothManager::adapterDeviceDiscoveryStateChangedCallback(int result, bt_adapter_device_discovery_state_e discovery_state, bt_adapter_device_discovery_info_s *discovery_info, void* user_data) noexcept{
        BluetoothManager& bluetoothManager = *static_cast<BluetoothManager*> (user_data);
        if(discovery_state == BT_ADAPTER_DEVICE_DISCOVERY_FOUND){
            bluetoothManager.addBluetoothDevice(*discovery_info);
        }
    }
    void BluetoothManager::addBluetoothDevice(bt_adapter_device_discovery_info_s& discovery_info) noexcept{
        BluetoothDevice bluetoothDevice;
        bluetoothDevice.set_remote_id(discovery_info.remote_address);
        bluetoothDevice.set_name(discovery_info.remote_name);
        bluetoothDevice.set_type(BluetoothDevice_Type_UNKNOWN);
        std::scoped_lock lock(discoveryDevices.mut);
        discoveryDevices.var.emplace_back(std::move(bluetoothDevice));
    }
    
    const std::vector<BluetoothDevice>& BluetoothManager::getDiscoveryDevices() const noexcept{
        return discoveryDevices.var;
    }

} // namespace btu

