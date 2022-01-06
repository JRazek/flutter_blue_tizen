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
        void setAddress(const std::string& address){ _address=address; }
        std::string address(){ return _address; }
        State getState() const noexcept{ return _state; }
        void setState(State state){ _state=state; }
        auto& protoBluetoothDevices(){ return _protoBluetoothDevices; }
        const auto& cProtoBluetoothDevices() const { return _protoBluetoothDevices; }
    };
};
#endif //BLUETOOTH_DEVICE_CONTROLLER_H