#include <GATT/BluetoothDescriptor.h>
#include <GATT/BluetoothCharacteristic.h>

namespace btGatt{
    BluetoothDescriptor::BluetoothDescriptor(bt_gatt_h handle, BluetoothCharacteristic& characteristic):
    _handle(handle),
    _characteristic(characteristic){}
    
    auto BluetoothDescriptor::toProtoDescriptor() const noexcept -> proto::gen::BluetoothDescriptor{
        proto::gen::BluetoothDescriptor proto;

        return proto;
    }
}