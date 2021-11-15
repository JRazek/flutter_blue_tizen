
#ifndef BLUETOOTH_UTILS_H
#define BLUETOOTH_UTILS_H

namespace btu{
    class BluetoothUtils{
        public:
        /**
         * @brief checks if the bluetooth is available on the device
         */
        static bool getBluetoothState() noexcept;
    };
} // namespace btu

#endif //BLUETOOTH_UTILS_H