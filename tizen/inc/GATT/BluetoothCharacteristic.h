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
        int _properties;

        std::vector<std::unique_ptr<BluetoothDescriptor>> _descriptors;
    public:

        BluetoothCharacteristic(bt_gatt_h handle, BluetoothService& service);
        auto toProtoCharacteristic() const noexcept -> proto::gen::BluetoothCharacteristic;
        auto cService() const noexcept -> const decltype(_service)&;
        auto UUID() const noexcept -> std::string;
    };
}
#endif //BLEUTOOTH_CHARACTERISTIC_H