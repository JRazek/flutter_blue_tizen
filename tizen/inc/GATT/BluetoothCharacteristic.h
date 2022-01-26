#ifndef BLEUTOOTH_CHARACTERISTIC_H
#define BLEUTOOTH_CHARACTERISTIC_H
#include <bluetooth.h>
#include <memory>
#include <vector>

#include <flutterblue.pb.h>
#include <GATT/BluetoothDescriptor.h>

namespace btGatt{
    class BluetoothService;

    class BluetoothCharacteristic{
        bt_gatt_h _handle;
        BluetoothService& _service;

        std::vector<BluetoothDescriptor> _descriptors;
    public:

        BluetoothCharacteristic(bt_gatt_h handle, BluetoothService& service);

        auto toProtoCharacteristic() const noexcept -> proto::gen::BluetoothCharacteristic;
    };
}
#endif //BLEUTOOTH_CHARACTERISTIC_H