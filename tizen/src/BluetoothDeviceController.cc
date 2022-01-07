#include <BluetoothDeviceController.h>
#include <Logger.h>
#include <BluetoothManager.h>

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
    auto BluetoothDeviceController::connectionStateCallback(int res, bool connected, const char* remote_address, void* user_data) noexcept -> void{
        if(!res){
            BluetoothManager& bluetoothManager = *static_cast<BluetoothManager*> (user_data);
            std::scoped_lock lock(bluetoothManager.bluetoothDevices().mut);
            auto& device=bluetoothManager.bluetoothDevices().var[remote_address];
            std::scoped_lock devLock(device.operationM);
            device.state()=(connected ? State::CONNECTED : State::CONNECTION_FAILED);
            Logger::log(LogLevel::DEBUG, "connected to device "+device.cAddress());
        }else{
            std::string err=get_error_message(res);
            Logger::log(LogLevel::ERROR, "device service search failed with " + err);
        }
    }
};