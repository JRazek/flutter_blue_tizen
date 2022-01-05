#ifndef BLUETOOTH_DEVICE_CONTROLLER_H
#define BLUETOOTH_DEVICE_CONTROLLER_H
#include <flutterblue.pb.h>
#include <condition_variable>

namespace btu{
    class BluetoothDeviceController{
    public:
        enum class State;
    private:
        BluetoothDevice _protoBluetoothDevice;
        State _state;
        std::string _address;
        int _scanCount;//number of duplicated values if allow duplicates==true
    public:
        std::condition_variable cv;
        enum class State{
            DEFAULT,
            SCANNED,
            CONNECTED,
        };
        
        BluetoothDeviceController(){
            _state=State::DEFAULT;
            _scanCount=0;
        }
        void setAddress(const std::string& address){ _address=address; }
        std::string address(){ return _address; }
        State getState() const noexcept{ return _state; }
        void setState(State state){ _state=state; }
        BluetoothDevice& protoBluetoothDevice(){ return _protoBluetoothDevice; }
        const BluetoothDevice& cProtoBluetoothDevice() const { return _protoBluetoothDevice; }
        int& scanCount(){ return _scanCount; }
        const int& cScanCount() const { return _scanCount; }
    };
};
#endif //BLUETOOTH_DEVICE_CONTROLLER_H