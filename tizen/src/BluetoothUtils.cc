#include "BluetoothUtils.h"
#include <system_info.h>

namespace btu{
    bool BluetoothUtils::getBluetoothState() noexcept{
        bool state;
        system_info_get_platform_bool("http://tizen.org/feature/network.bluetooth", &state);
        return state;
    }
} // namespace btu
