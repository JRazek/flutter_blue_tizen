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
        std::atomic<bool> _valueFetched=false;

        std::vector<std::shared_ptr<BluetoothDescriptor>> _descriptors;
    public:

        BluetoothCharacteristic(bt_gatt_h handle, BluetoothService& service);
        auto toProtoCharacteristic() const noexcept -> proto::gen::BluetoothCharacteristic;
        auto cService() const noexcept -> const decltype(_service)&;
        auto UUID() const noexcept -> std::string;
        auto value() const noexcept -> std::string;
        auto getDescriptor(const std::string& uuid) -> std::shared_ptr<BluetoothDescriptor>;
        auto read(const std::function<void(BluetoothCharacteristic&)>& callback) -> void;
    };
}
#endif //BLEUTOOTH_CHARACTERISTIC_H