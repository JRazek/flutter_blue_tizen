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
        using NotifyCallback=std::function<void(const BluetoothCharacteristic&)>;
        bt_gatt_h _handle;
        BluetoothService& _service;

        std::vector<std::shared_ptr<BluetoothDescriptor>> _descriptors;

        std::unique_ptr<NotifyCallback> _notifyCallback;



    public:

        BluetoothCharacteristic(bt_gatt_h handle, BluetoothService& service) noexcept;
        ~BluetoothCharacteristic() noexcept;
        auto toProtoCharacteristic() const noexcept -> proto::gen::BluetoothCharacteristic;
        auto cService() const noexcept -> const decltype(_service)&;
        auto UUID() const noexcept -> std::string;
        auto value() const noexcept -> std::string;
        auto getDescriptor(const std::string& uuid) -> std::shared_ptr<BluetoothDescriptor>;
        auto read(const std::function<void(const BluetoothCharacteristic&)>& callback) -> void;
        auto write(const std::string value, bool withoutResponse, const std::function<void(bool success, const BluetoothCharacteristic&)>& callback) -> void;
        auto properties() const noexcept -> int;
        
        auto setNotifyCallback(const NotifyCallback& callback) -> void;
        void unsetNotifyCallback();
    };
}
#endif //BLEUTOOTH_CHARACTERISTIC_H