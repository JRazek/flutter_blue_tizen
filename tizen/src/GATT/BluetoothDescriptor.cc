#include <GATT/BluetoothDescriptor.h>

namespace btGatt{
    BluetoothDescriptor::BluetoothDescriptor(bt_gatt_h handle, std::weak_ptr<BluetoothCharacteristic> characteristic):
    _handle(handle),
    _characteristic(characteristic){}
}