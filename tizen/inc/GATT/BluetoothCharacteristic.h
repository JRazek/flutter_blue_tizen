#ifndef BLEUTOOTH_CHARACTERISTIC_H
#define BLEUTOOTH_CHARACTERISTIC_H
#include <bluetooth.h>
#include <memory>
#include <vector>

namespace btGatt{
    class BluetoothService;
    class BluetoothDescriptor;
    class BluetoothCharacteristic{
        bt_gatt_h _handle;
        std::weak_ptr<BluetoothService> _service;

        std::vector<BluetoothDescriptor> _descriptors;
    public:

        BluetoothCharacteristic(bt_gatt_h handle, std::weak_ptr<BluetoothService> service);
    };
}
#endif //BLEUTOOTH_CHARACTERISTIC_H