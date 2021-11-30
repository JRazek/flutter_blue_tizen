#ifndef BLUETOOTH_UTILS_H
#define BLUETOOTH_UTILS_H

#include <bluetooth.h>

#include <flutter/method_channel.h>
#include <flutter/encodable_value.h>

#include <vector>
#include <mutex>
#include <memory>
#include <condition_variable>

class BluetoothState;
class BluetoothDevice;
class ScanResult;
class ScanSettings;

namespace btu{
    template<typename T>
    struct SafeType{
        T var;
        std::mutex mut;
    };
    template<typename T>
    struct ConditionVariable{
        T state;
        std::condition_variable cv;
    };
    using MethodChannel = flutter::MethodChannel<flutter::EncodableValue>;
    class BluetoothManager{
        SafeType<bt_adapter_state_e> adapterState;
        SafeType<std::vector<BluetoothDevice>> connectedDevices;
        std::shared_ptr<MethodChannel> methodChannel;

        SafeType<bool> scanningInProgress{false};

    public:

        BluetoothManager(std::shared_ptr<MethodChannel> _methodChannel) noexcept;
        virtual ~BluetoothManager() noexcept;
        
        /**
         * @brief checks if the bluetooth is available on the device
         */
        static bool getBluetoothAvailabilityLE() noexcept;
        
        void startBluetoothDeviceScanLE(const ScanSettings& scanSettings) noexcept;
        
        void stopBluetoothDeviceScanLE() noexcept;
        
        static void adapterDeviceDiscoveryStateChangedCallbackLE(int result, bt_adapter_le_device_scan_result_info_s *discovery_info, void *user_data) noexcept;

        bool adapterIsScanningLE() const noexcept;

        /**
         * @brief thread safe
         * 
         * @param discovery_info 
         */
        void notifyDiscoveryResultLE(const bt_adapter_le_device_scan_result_info_s& discovery_info) noexcept;


        //////////////////////////
        BluetoothState getBluetoothState() const noexcept;

        static void adapterStateChangedCallback(int result, bt_adapter_state_e adapter_state, void* user_data) noexcept;
        void setAdapterState(bt_adapter_state_e state) noexcept;

        SafeType<std::vector<BluetoothDevice>>& getConnectedDevicesLE() noexcept;
    };
} // namespace btu

#endif //BLUETOOTH_UTILS_H