#ifndef BLUETOOTH_DEVICE_CONTROLLER_H
#define BLUETOOTH_DEVICE_CONTROLLER_H
#include <flutterblue.pb.h>

#include <condition_variable>

#include <bluetooth.h>

namespace btu{
    class NotificationsHandler;
    class BluetoothService;
    class BluetoothDeviceController{
    public:
        enum class State;
    private:
        /**
         * @brief all attributes are depentent on this mutex
         * 
         */
        std::mutex operationM;

        std::vector<proto::gen::BluetoothDevice> _protoBluetoothDevices;

        std::string _address;
        std::condition_variable cv;

        NotificationsHandler& _notificationsHandler;

    public:
        enum class State{
            CONNECTED,
            DISCONNECTED,
        };

        BluetoothDeviceController(const std::string& address, NotificationsHandler& notificationsHandler) noexcept;
        BluetoothDeviceController(const char* address, NotificationsHandler& notificationsHandler) noexcept;
        ~BluetoothDeviceController() noexcept;

        BluetoothDeviceController(const BluetoothDeviceController& address)=delete;
        
        auto cAddress() const noexcept -> const decltype(_address)&;
        auto state() noexcept -> State;
        auto protoBluetoothDevices() noexcept -> decltype(_protoBluetoothDevices)&;
        auto cProtoBluetoothDevices() const noexcept -> const decltype(_protoBluetoothDevices)&;

        auto connect(const proto::gen::ConnectRequest& connReq) noexcept -> void;
        auto disconnect() noexcept -> void;

        static auto connectionStateCallback(int result, bool connected, const char* remote_address, void* user_data) noexcept -> void;
        static auto getGattClient(const std::string& address) noexcept -> bt_gatt_client_h;
        static auto destroyGattClientIfExists(const std::string& address) noexcept -> void;

        auto discoverServices() noexcept -> void;
    };
};
#endif //BLUETOOTH_DEVICE_CONTROLLER_H