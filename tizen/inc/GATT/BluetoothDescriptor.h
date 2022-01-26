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
    public:
        BluetoothDescriptor(bt_gatt_h handle, BluetoothCharacteristic& characteristic);

        auto toProtoDescriptor() const noexcept -> proto::gen::BluetoothDescriptor;
    };
}
#endif //BLUETOOTH_DESCRIPTOR_H