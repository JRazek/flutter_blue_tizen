#ifndef BLUETOOTH_MANAGER_H
#define BLUETOOTH_MANAGER_H

#include <bluetooth.h>

#include <flutter/method_channel.h>
#include <flutter/encodable_value.h>

#include <vector>
#include <mutex>
#include <memory>
#include <condition_variable>
#include <unordered_set>

#include <flutterblue.pb.h>
#include <Utils.h>
#include <BluetoothDeviceController.h>

namespace btu{
    using MethodChannel = flutter::MethodChannel<flutter::EncodableValue>;
    class BluetoothManager{
         SafeType<bt_adapter_state_e> adapterState;

         /**
          * @brief key - MAC address of the device
          * 
          */
         SafeType<std::unordered_map<std::string, BluetoothDeviceController>> bluetoothDevices;


         std::shared_ptr<MethodChannel> methodChannel;

         SafeType<bt_advertiser_h> advertisingHandler={nullptr};
    public:

         SafeType<bool> scanAllowDuplicates;
         BluetoothManager(std::shared_ptr<MethodChannel> _methodChannel) noexcept;
         virtual ~BluetoothManager() noexcept;
         BluetoothManager(const BluetoothManager& bluetoothManager)=delete;
         
         /**
            * @brief checks if the bluetooth is available on the device
            */
         static bool getBluetoothAvailabilityLE() noexcept;
         
         void startBluetoothDeviceScanLE(const ScanSettings& scanSettings) noexcept;
         
         /**
            * @brief this is an atomic function
            * 
            */
         void stopBluetoothDeviceScanLE() noexcept;
         
         static void scanCallback(int result, bt_adapter_le_device_scan_result_info_s *discovery_info, void *user_data);
         
         static void adapterAdvertisingStateChangedCallbackLE(int result, bt_advertiser_h advertiser, bt_adapter_le_advertising_state_e adv_state, void *user_data);

         bool adapterIsScanningLE() const noexcept;

         /**
            * @brief 
            * 
               NOT TESTED!
            */
         void connect(const ConnectRequest& connRequest) noexcept;

         /**
            * @brief 
            * 
               NOT TESTED!
            */
         void disconnect(const std::string& deviceID) noexcept;

         /**
            * @brief this is an atomic callback function
               NOT TESTED!
            */
         static void deviceConnectedCallback(int result, bt_device_info_s* device_info, void* user_data) noexcept;

         /**
            * @brief 
            * 
               NOT TESTED!
            */
         static void deviceDisconnectedCallback(int result, char* remote_address, void* user_data) noexcept;

         /**
            * @brief 
            * 
               NOT TESTED!
            */
         void serviceSearch(const BluetoothDevice& bluetoothDevice) noexcept;

         /**
            * @brief 
            * 
               NOT TESTED!
            */
         static void serviceSearchCallback(int result, bt_device_sdp_info_s* device_info, void* user_data) noexcept;

         void startAdvertising() noexcept;

         //////////////////////////
         BluetoothState getBluetoothState() const noexcept;

         static void adapterStateChangedCallback(int result, bt_adapter_state_e adapter_state, void* user_data) noexcept;
         void setAdapterState(bt_adapter_state_e state) noexcept;

         std::vector<BluetoothDevice> getConnectedProtoBluetoothDevices() noexcept;
    };
} // namespace btu

#endif //BLUETOOTH_MANAGER_H