#ifndef BLUETOOTH_DEVICE_CONTROLLER_H
#define BLUETOOTH_DEVICE_CONTROLLER_H
#include <flutterblue.pb.h>
#include <Utils.h>

#include <condition_variable>

#include <bluetooth.h>

namespace btu{
    class BluetoothDeviceController{
    public:
        enum class State;
    private:
        std::vector<BluetoothDevice> _protoBluetoothDevices;
        State _state;
        std::string _address;

        SafeType<std::pair<bt_gatt_h, bool>> gattServerHandle;//handle, created/not created
    public:
        enum class State{
            DEFAULT,
            SCANNED,
            CONNECTED,
        };
        std::condition_variable cv;
        
        BluetoothDeviceController() noexcept;
        ~BluetoothDeviceController() noexcept;

        auto address() noexcept -> decltype(_address)&;
        auto cAddress() const noexcept -> const decltype(_address)&;
        auto state() noexcept -> decltype(_state)&;
        auto cState() const noexcept -> const decltype(_state)&;
        auto protoBluetoothDevices() noexcept -> decltype(_protoBluetoothDevices)&;
        auto cProtoBluetoothDevices() const noexcept -> const decltype(_protoBluetoothDevices)&;
        auto connect() -> void;
        auto disconnect() -> void
    };
};
#endif //BLUETOOTH_DEVICE_CONTROLLER_H