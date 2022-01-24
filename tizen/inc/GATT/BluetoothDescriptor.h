#ifndef BLUETOOTH_DESCRIPTOR_H
#define BLUETOOTH_DESCRIPTOR_H

#include <bluetooth.h>

#include <memory>

namespace btGatt{
    class BluetoothService;
    class BluetoothDescriptor{
        bt_gatt_h _handle;
        std::weak_ptr<BluetoothCharacteristic> _characteristic;
    public:
        BluetoothDescriptor(bt_gatt_h handle, std::weak_ptr<BluetoothService> characteristic);
    };
}
#endif //BLUETOOTH_DESCRIPTOR_H