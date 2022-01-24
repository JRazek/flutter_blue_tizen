#ifndef BLEUTOOTH_CHARACTERISTIC_H
#define BLEUTOOTH_CHARACTERISTIC_H
#include <BluetoothService.h>
namespace btu{
    class BluetoothCharacteristic{
        bt_gatt_h _handle;
        std::weak_ptr<BluetoothService> _service;
        BluetoothCharacteristic(bt_gatt_h handle, std::weak_ptr<BluetoothService> service):
        _handle(handle),
        _service(service){}
    };
}
#endif //BLEUTOOTH_CHARACTERISTIC_H