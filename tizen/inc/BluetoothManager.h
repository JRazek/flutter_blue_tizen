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
          virtual ~BluetoothManager() noexcept=default;
          BluetoothManager(const BluetoothManager& bluetoothManager)=delete;
          
          auto startBluetoothDeviceScanLE(const proto::gen::ScanSettings& scanSettings) noexcept -> void;
          auto stopBluetoothDeviceScanLE() noexcept -> void;
          auto connect(const proto::gen::ConnectRequest& connRequest) noexcept -> void;
          auto disconnect(const std::string& deviceID) noexcept -> void;
          auto bluetoothState() const noexcept -> proto::gen::BluetoothState;
          auto getConnectedProtoBluetoothDevices() noexcept -> std::vector<proto::gen::BluetoothDevice>;
          auto bluetoothDevices() noexcept -> decltype(_bluetoothDevices)&;
          auto readCharacteristic(const proto::gen::ReadCharacteristicRequest& request) -> void;
          auto readDescriptor(const proto::gen::ReadDescriptorRequest& request) -> void;
          auto writeCharacteristic(const proto::gen::WriteCharacteristicRequest& request) -> void;
          auto writeDescriptor(const proto::gen::WriteDescriptorRequest& request) -> void;

          auto locateCharacteristic(const std::string& remoteID, const std::string& primaryUUID, const std::string& secondaryUUID, const std::string& characteristicUUID) -> std::shared_ptr<btGatt::BluetoothCharacteristic>;
          auto locateDescriptor(const std::string& remoteID, const std::string& primaryUUID, const std::string& secondaryUUID, const std::string& characteristicUUID, const std::string& descriptorUUID) -> std::shared_ptr<btGatt::BluetoothDescriptor>;
          static auto isBLEAvailable() noexcept -> bool;
          static auto scanCallback(int result, bt_adapter_le_device_scan_result_info_s* discovery_info, void* user_data) noexcept -> void;
          static auto adapterStateChangedCallback(int result, bt_adapter_state_e adapter_state, void* user_data) noexcept -> void;
     };
} // namespace btu

#endif //BLUETOOTH_MANAGER_H