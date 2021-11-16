#ifndef BLUETOOTH_UTILS_H
#define BLUETOOTH_UTILS_H

#include "BluetoothState.h"

namespace btu{
    class BluetoothManager{
    public:

        BluetoothManager() noexcept;
        virtual ~BluetoothManager() noexcept;

        /**
         * @brief checks if the bluetooth is available on the device
         */
        bool getBluetoothAvailability() const noexcept;
        
        void setBluetoothState(BluetoothState bluetoothState) noexcept;
        BluetoothState getBluetoothState() const noexcept;
    };
} // namespace btu

#endif //BLUETOOTH_UTILS_H