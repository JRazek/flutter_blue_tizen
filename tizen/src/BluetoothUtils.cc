#include "BluetoothUtils.h"
#include <system_info.h>

namespace btu{
    bool BluetoothUtils::getBluetoothAvailability() const noexcept{
        bool state;
        system_info_get_platform_bool("http://tizen.org/feature/network.bluetooth", &state);
        return state;
    }

    BluetoothState BluetoothUtils::getBluetoothState() const noexcept{
        //[TODO] where are the states in docs??
    }

} // namespace btu
