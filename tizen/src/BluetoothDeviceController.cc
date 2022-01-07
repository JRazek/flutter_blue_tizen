#include <BluetoothDeviceController.h>
#include <Logger.h>

#include <mutex>
namespace btu {
    using namespace btlog;
    BluetoothDeviceController::BluetoothDeviceController() noexcept:
    _state(State::DEFAULT){}

    BluetoothDeviceController::~BluetoothDeviceController() noexcept{
        if(isConnected) disconnect();

    }

    auto BluetoothDeviceController::address() noexcept -> decltype(_address)& { return _address; }
    auto BluetoothDeviceController::cAddress() const noexcept -> const decltype(_address)& { return _address; }
    auto BluetoothDeviceController::state() noexcept -> decltype(_state)& { return _state; }
    auto BluetoothDeviceController::cState() const noexcept -> const decltype(_state)& { return _state; }
    auto BluetoothDeviceController::protoBluetoothDevices() noexcept -> decltype(_protoBluetoothDevices)& { return _protoBluetoothDevices; }
    auto BluetoothDeviceController::cProtoBluetoothDevices() const noexcept -> const decltype(_protoBluetoothDevices)& { return _protoBluetoothDevices; }

    auto BluetoothDeviceController::connect(const ConnectRequest& connReq) noexcept -> void {
        std::scoped_lock lock(operationM);
        if(!isConnected && _state==State::SCANNED){
            int res=bt_gatt_connect(_address.c_str(), connReq.android_auto_connect());
            isConnected=true;
        }else{
            Logger::log(LogLevel::ERROR, "already connected to device "+_address);
        }
    }
    auto BluetoothDeviceController::disconnect() noexcept -> void {
        std::scoped_lock lock(operationM);
        if(isConnected){
            isConnected=false;

        }else{
            Logger::log(LogLevel::ERROR, "cannot disconnect. Device not connected "+_address);
        }
    }
    auto BluetoothDeviceController::connectionStateCallback(int result, bool connected, const char* remote_address, void* user_data) noexcept -> void{
        //remote_address->this... somehow...
    }
};