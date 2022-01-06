#ifndef BLUETOOTH_DEVICE_CONTROLLER_H
#define BLUETOOTH_DEVICE_CONTROLLER_H
#include <flutterblue.pb.h>
#include <condition_variable>

namespace btu{
    class BluetoothDeviceController{
    public:
        enum class State;
    private:
        std::vector<BluetoothDevice> _protoBluetoothDevices;
        State _state;
        std::string _address;
    public:
        std::condition_variable cv;
        enum class State{
            DEFAULT,
            SCANNED,
            CONNECTED,
        };
        
        BluetoothDeviceController(){
            _state=State::DEFAULT;
        }
        
        auto& address() noexcept { return _address; }
        const auto& cAddress() const noexcept { return _address; }
        auto& state() noexcept{ return _state; }
        const auto& cState() const noexcept{ return _state; }
        auto& protoBluetoothDevices(){ return _protoBluetoothDevices; }
        const auto& cProtoBluetoothDevices() const noexcept { return _protoBluetoothDevices; }
    };
};
#endif //BLUETOOTH_DEVICE_CONTROLLER_H