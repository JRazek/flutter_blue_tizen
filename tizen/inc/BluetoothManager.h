#ifndef BLUETOOTH_UTILS_H
#define BLUETOOTH_UTILS_H

#include <bluetooth.h>

#include <flutter/method_channel.h>
#include <flutter/encodable_value.h>

#include <vector>
#include <mutex>
#include <memory>
#include <condition_variable>
#include <unordered_set>

#include <flutterblue.pb.h>

namespace btu{
    template<typename T>
    struct SafeType{
        T var;
        std::mutex mut;
    };

    using MethodChannel = flutter::MethodChannel<flutter::EncodableValue>;
    class BluetoothManager{
        SafeType<bt_adapter_state_e> adapterState;

        SafeType<std::unordered_map<std::string, BluetoothDevice>> connectedDevices;

        SafeType<std::unordered_map<std::string, std::pair<std::condition_variable, bool>>> pendingConnectionRequests;

        std::shared_ptr<MethodChannel> methodChannel;

        SafeType<bool> scanningInProgress{false};

        SafeType<std::unordered_map<std::string, BluetoothDevice>> discoveredDevicesAddresses;
    public:

        BluetoothManager(std::shared_ptr<MethodChannel> _methodChannel) noexcept;
        virtual ~BluetoothManager() noexcept;
        
        /**
         * @brief checks if the bluetooth is available on the device
         */
        static bool getBluetoothAvailabilityLE() noexcept;
        
        void startBluetoothDeviceScanLE(const ScanSettings& scanSettings) noexcept;
        
        void stopBluetoothDeviceScanLE() noexcept;
        
        static void adapterDeviceDiscoveryStateChangedCallbackLE(int result, bt_adapter_le_device_scan_result_info_s *discovery_info, void *user_data);

        bool adapterIsScanningLE() const noexcept;

        void connect(const ConnectRequest& connRequest) noexcept;

        static void deviceConnectedCallback(int result, bt_device_info_s *device_info, void *user_data) noexcept;

        /**
         * @brief thread safe
         * 
         * @param discovery_info 
         */
        void notifyDiscoveryResultLE(const bt_adapter_le_device_scan_result_info_s& discovery_info);


        //////////////////////////
        BluetoothState getBluetoothState() const noexcept;

        static void adapterStateChangedCallback(int result, bt_adapter_state_e adapter_state, void* user_data) noexcept;
        void setAdapterState(bt_adapter_state_e state) noexcept;

        SafeType<std::unordered_map<std::string, BluetoothDevice>>& getConnectedDevicesLE() noexcept;
    };
} // namespace btu

#endif //BLUETOOTH_UTILS_H