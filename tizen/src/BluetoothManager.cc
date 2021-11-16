#include <BluetoothState.h>
#include <BluetoothManager.h>

#include <system_info.h>
#include <Log.h>
#include <bluetooth.h>

namespace btu{

    BluetoothManager::BluetoothManager() noexcept{
        if(bt_initialize() != BT_ERROR_NONE){
            LOG(DLOG_ERROR, "[bt_initialize] failed");
        }
        //[todo] error handing

        int res = bt_adapter_set_state_changed_cb(BluetoothManager::adapterStateChangedCallback, this);
        if(res != BT_ERROR_NONE){
            LOG(DLOG_ERROR, "could not set callback function!");
        }
        res = bt_adapter_set_device_discovery_state_changed_cb(BluetoothManager::adapterDeviceDiscoveryStateChangedCallback, this);
    }

    BluetoothManager::~BluetoothManager() noexcept{
        if(bt_deinitialize() != BT_ERROR_NONE){
            LOG(DLOG_ERROR, "[bt_deinitialize] failed");
        }
    }

    bool BluetoothManager::getBluetoothAvailability() noexcept{
        bool state;
        system_info_get_platform_bool("http://tizen.org/feature/network.bluetooth", &state);
        return state;
    }

    BluetoothState BluetoothManager::getBluetoothState() const noexcept{
        /* Check whether the Bluetooth adapter is enabled */
        bt_adapter_state_e adapter_state;
        int ret = bt_adapter_get_state(&adapter_state);
        BluetoothState bluetoothState;
        if(ret == BT_ERROR_NONE){
            if(adapter_state == BT_ADAPTER_ENABLED)
                bluetoothState = BluetoothState::ON;
            else
                bluetoothState = BluetoothState::OFF;
        }else if(ret == BT_ERROR_NOT_INITIALIZED){
            bluetoothState = BluetoothState::UNINITIALIZED;
        }else{
            bluetoothState = BluetoothState::UNKNOWN;
        }
        return bluetoothState;
    }

    void BluetoothManager::adapterStateChangedCallback(int result, bt_adapter_state_e adapter_state, void* user_data) noexcept{
        BluetoothManager& bluetoothManager = *static_cast<BluetoothManager*> (user_data);
        bluetoothManager.setAdapterState(adapter_state);
    }
    void BluetoothManager::setAdapterState(bt_adapter_state_e _state) noexcept{
        std::scoped_lock(adapterState.mut);
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
        std::scoped_lock(devices.mut);
        devices.var.emplace_back(std::move(discovery_info));
    }
} // namespace btu
