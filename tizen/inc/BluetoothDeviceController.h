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
        std::mutex operationM;
        std::condition_variable cv;
    public:
        enum class State{
            DEFAULT,
            SCANNED,
            CONNECTED,
            CONNECTING,
            DISCONNECTED,
            DISCONNECTING,
        };

        
        BluetoothDeviceController() noexcept;
        ~BluetoothDeviceController() noexcept;

        auto address() noexcept -> decltype(_address)&;
        auto cAddress() const noexcept -> const decltype(_address)&;
        auto state() noexcept -> decltype(_state)&;
        auto cState() const noexcept -> const decltype(_state)&;
        auto protoBluetoothDevices() noexcept -> decltype(_protoBluetoothDevices)&;
        auto cProtoBluetoothDevices() const noexcept -> const decltype(_protoBluetoothDevices)&;
        auto connect(const ConnectRequest& connReq) noexcept -> void;
        auto disconnect() noexcept -> void;

        static auto connectionStateCallback(int result, bool connected, const char* remote_address, void* user_data) noexcept -> void;
    };
};
#endif //BLUETOOTH_DEVICE_CONTROLLER_H