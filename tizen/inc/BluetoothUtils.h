#ifndef BLUETOOTH_UTILS_H
#define BLUETOOTH_UTILS_H

#include "BluetoothState.h"

namespace btu{
    class BluetoothUtils{
    public:

        BluetoothUtils() noexcept = default;

        /**
         * @brief checks if the bluetooth is available on the device
         */
        bool getBluetoothAvailability() const noexcept;

        BluetoothState getBluetoothState() const noexcept;
    };
} // namespace btu

#endif //BLUETOOTH_UTILS_H