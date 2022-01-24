#ifndef BLUETOOTH_DESCRIPTOR_H
#define BLUETOOTH_DESCRIPTOR_H

#include <BluetoothCharacteristic.h>
namespace btu{
    class BluetoothDescriptor{
        bt_gatt_h _handle;
        std::weak_ptr<BluetoothCharacteristic> _characteristic;
        
        BluetoothDescriptor(bt_gatt_h handle, std::weak_ptr<BluetoothService> characteristic):
        _handle(handle),
        _characteristic(characteristic){}
    };
}
#endif //BLUETOOTH_DESCRIPTOR_H