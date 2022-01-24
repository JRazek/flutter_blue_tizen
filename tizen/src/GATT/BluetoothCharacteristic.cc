#include <GATT/BluetoothCharacteristic.h>

namespace btGatt{
    BluetoothCharacteristic::BluetoothCharacteristic(bt_gatt_h handle, std::weak_ptr<BluetoothService> service):
    _handle(handle),
    _service(service){}
}