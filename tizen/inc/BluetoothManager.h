#ifndef BLUETOOTH_UTILS_H
#define BLUETOOTH_UTILS_H

#include <bluetooth.h>

#include <flutter/method_channel.h>
#include <flutter/encodable_value.h>

#include <vector>
#include <mutex>
#include <memory>

class BluetoothState;
class BluetoothDevice;
class ScanResult;

namespace btu{
    template<typename T>
    struct SafeType{
        T var;
        std::mutex mut;
    };
    using MethodChannel = flutter::MethodChannel<flutter::EncodableValue>;
    class BluetoothManager{
        SafeType<bt_adapter_state_e> adapterState;
        SafeType<std::vector<BluetoothDevice>> connectedDevices;
        std::shared_ptr<MethodChannel> methodChannel;
    public:

        BluetoothManager(std::shared_ptr<MethodChannel> _methodChannel) noexcept;
        virtual ~BluetoothManager() noexcept;
        
        /**
         * @brief checks if the bluetooth is available on the device
         */
        static bool getBluetoothAvailability() noexcept;
        

        void startBluetoothDeviceDiscovery() const noexcept;
        void stopBluetoothDeviceDiscovery() const noexcept;
        static void adapterDeviceDiscoveryStateChangedCallback(int result, bt_adapter_device_discovery_state_e discovery_state, bt_adapter_device_discovery_info_s *discovery_info, void* user_data) noexcept;

        /**
         * @brief thread safe
         * 
         * @param discovery_info 
         */
        void addDiscoveryResult(bt_adapter_device_discovery_info_s& discovery_info) noexcept;


        //////////////////////////
        BluetoothState getBluetoothState() const noexcept;

        static void adapterStateChangedCallback(int result, bt_adapter_state_e adapter_state, void* user_data) noexcept;
        void setAdapterState(bt_adapter_state_e state) noexcept;

        SafeType<std::vector<BluetoothDevice>>& getConnectedDevices() noexcept;
    };
} // namespace btu

#endif //BLUETOOTH_UTILS_H