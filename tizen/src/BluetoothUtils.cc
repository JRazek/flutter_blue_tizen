#include <system_info.h>
#include <Log.h>
#include <bluetooth.h>

#include <BluetoothState.h>
#include <BluetoothUtils.h>

namespace btu{
    bool BluetoothManager::getBluetoothAvailability() const noexcept{
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
            else if(adapter_state == BT_ADAPTER_DISABLED)
                bluetoothState = BluetoothState::OFF;
        }else if(ret == BT_ERROR_NOT_INITIALIZED){
            bluetoothState = BluetoothState::UNINITIALIZED;
        }else{
            bluetoothState = BluetoothState::UNKNOWN;
        }
        return bluetoothState;
    }

    BluetoothManager::BluetoothManager() noexcept{
        if(bt_initialize() != BT_ERROR_NONE){
            LOG(DLOG_ERROR, "[bt_initialize] failed");
        }
        //[todo] error handing
    }
    BluetoothManager::~BluetoothManager() noexcept{
        if(bt_deinitialize() != BT_ERROR_NONE){
            LOG(DLOG_ERROR, "[bt_deinitialize] failed");
        }
    }

} // namespace btu
