#include <BluetoothDeviceController.h>
#include <Logger.h>
#include <BluetoothManager.h>

#include <mutex>
namespace btu {
    using namespace btlog;
    BluetoothDeviceController::BluetoothDeviceController() noexcept:
    _state(State::DEFAULT){}

    BluetoothDeviceController::~BluetoothDeviceController() noexcept{
        if(_state==State::CONNECTED) disconnect(); 
    }

    auto BluetoothDeviceController::address() noexcept -> decltype(_address)& { return _address; }
    auto BluetoothDeviceController::cAddress() const noexcept -> const decltype(_address)& { return _address; }
    auto BluetoothDeviceController::state() noexcept -> decltype(_state)& { return _state; }
    auto BluetoothDeviceController::cState() const noexcept -> const decltype(_state)& { return _state; }
    auto BluetoothDeviceController::protoBluetoothDevices() noexcept -> decltype(_protoBluetoothDevices)& { return _protoBluetoothDevices; }
    auto BluetoothDeviceController::cProtoBluetoothDevices() const noexcept -> const decltype(_protoBluetoothDevices)& { return _protoBluetoothDevices; }

    auto BluetoothDeviceController::connect(const ConnectRequest& connReq) noexcept -> void {
        using namespace std::literals;
        std::unique_lock lock(operationM);
        if(_state==State::SCANNED){
            Logger::log(LogLevel::DEBUG, "connecting to device "+_address);
            int res=bt_gatt_connect(_address.c_str(), connReq.android_auto_connect());
            _state=State::CONNECTING;

        }else{
            Logger::log(LogLevel::ERROR, "already connected to device "+_address);
        }
    }
    auto BluetoothDeviceController::disconnect() noexcept -> void {
        std::unique_lock lock(operationM);
        if(_state==State::CONNECTED || _state==State::CONNECTING){
            _state=State::DISCONNECTING;
            int res=bt_gatt_disconnect(_address.c_str());
        }else{
            Logger::log(LogLevel::ERROR, "cannot disconnect. Device not connected "+_address);
        }
    }
    auto BluetoothDeviceController::connectionStateCallback(int res, bool connected, const char* remote_address, void* user_data) noexcept -> void{
        Logger::log(LogLevel::DEBUG, "callback called!");
        BluetoothManager& bluetoothManager = *static_cast<BluetoothManager*> (user_data);
        std::scoped_lock lock(bluetoothManager.bluetoothDevices().mut);
        auto& device=bluetoothManager.bluetoothDevices().var[remote_address];
        std::scoped_lock devLock(device.operationM);
        if(!res){
            if(connected && device.cState()==State::CONNECTING){
                device.state()=State::CONNECTED;
                Logger::log(LogLevel::DEBUG, "connected to device "+device.cAddress());
            }else{
                device.state()=State::DISCONNECTED;
                Logger::log(LogLevel::DEBUG, "disconnected from device"+device.cAddress());
            }
        }else{
            std::string err=get_error_message(res);
            Logger::log(LogLevel::ERROR, "connectionStateCallback " + err);
        }
    }
};