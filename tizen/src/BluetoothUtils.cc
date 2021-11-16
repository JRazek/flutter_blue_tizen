#include "BluetoothUtils.h"
#include <system_info.h>
#include <Log.h>
#include <bluetooth.h>

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
        switch (ret){
        case BT_ERROR_NONE:
            bluetoothState = BluetoothState::ON;
            break;
        case BT_ERROR_NOT_INITIALIZED:
            bluetoothState = BluetoothState::UNINITIALIZED;
            break;
        case BT_ERROR_NOT_ENABLED:
            bluetoothState = BluetoothState::OFF;
            break;
        default:
            LOG(DLOG_ERROR, "[bt_adapter_get_state] failed");
            bluetoothState = BluetoothState::UNKNOWN;
            break;
        }
    }

    BluetoothManager::BluetoothManager() noexcept{
        if(bt_initialize() != 0){
            LOG(DLOG_ERROR, "[bt_initialize] failed");
        }
        //[todo] error handing
    }
    BluetoothManager::~BluetoothManager() noexcept{
        if(bt_deinitialize() != 0){
            LOG(DLOG_ERROR, "[bt_deinitialize] failed");
        }
    }

} // namespace btu
