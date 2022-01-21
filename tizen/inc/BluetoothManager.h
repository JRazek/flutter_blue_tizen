#ifndef BLUETOOTH_MANAGER_H
#define BLUETOOTH_MANAGER_H

#include <bluetooth.h>

#include <vector>
#include <mutex>
#include <memory>
#include <condition_variable>
#include <unordered_set>

#include <Utils.h>
#include <NotificationsHandler.h>

namespace btu{
     class BluetoothDeviceController;
     class BluetoothManager{
          SafeType<bt_adapter_state_e> adapterState;
          /**
          * @brief key - MAC address of the device
          */
          SafeType<std::unordered_map<std::string, std::shared_ptr<BluetoothDeviceController>>> _bluetoothDevices;

          NotificationsHandler& _notificationsHandler;
          SafeType<bool> _scanAllowDuplicates;

     public:
          
          BluetoothManager(NotificationsHandler& notificationsHandler) noexcept;
          virtual ~BluetoothManager() noexcept;
          BluetoothManager(const BluetoothManager& bluetoothManager)=delete;
          
          auto startBluetoothDeviceScanLE(const ScanSettings& scanSettings) noexcept -> void;
          auto stopBluetoothDeviceScanLE() noexcept -> void;
          auto connect(const ConnectRequest& connRequest) noexcept -> void;
          auto disconnect(const std::string& deviceID) noexcept -> void;
          auto serviceSearch(const BluetoothDevice& bluetoothDevice) noexcept -> void;
          auto bluetoothState() const noexcept -> BluetoothState;
          auto getConnectedProtoBluetoothDevices() noexcept -> std::vector<BluetoothDevice>;
          auto bluetoothDevices() noexcept -> decltype(_bluetoothDevices)&;

          static auto isBLEAvailable() noexcept -> bool;
          static auto scanCallback(int result, bt_adapter_le_device_scan_result_info_s* discovery_info, void* user_data) noexcept -> void;
          static auto adapterStateChangedCallback(int result, bt_adapter_state_e adapter_state, void* user_data) noexcept -> void;
     };
} // namespace btu

#endif //BLUETOOTH_MANAGER_H