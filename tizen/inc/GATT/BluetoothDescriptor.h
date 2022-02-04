#ifndef BLUETOOTH_DESCRIPTOR_H
#define BLUETOOTH_DESCRIPTOR_H

#include <bluetooth.h>

#include <memory>

#include <flutterblue.pb.h>

namespace btGatt{
    class BluetoothCharacteristic;
    class BluetoothDescriptor{

        bt_gatt_h _handle;
        BluetoothCharacteristic& _characteristic;
        std::atomic<bool> _valueFetched=false;

    public:
        BluetoothDescriptor(bt_gatt_h handle, BluetoothCharacteristic& characteristic);
        auto toProtoDescriptor() const noexcept -> proto::gen::BluetoothDescriptor;
        auto UUID() const noexcept -> std::string;
        auto value() const noexcept -> std::string;
        auto read(const std::function<void(const BluetoothDescriptor&)>& callback) -> void;
        auto write(const std::string value, bool withResponse, const std::function<void(bool success, const BluetoothCharacteristic&)>& callback) -> void;
        auto cCharacteristic() const noexcept -> const BluetoothCharacteristic&;
    };
}
#endif //BLUETOOTH_DESCRIPTOR_H