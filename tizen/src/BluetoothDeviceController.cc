#include <BluetoothDeviceController.h>
#include <Logger.h>
#include <BluetoothManager.h>

#include <mutex>
namespace btu {
    using namespace btlog;
    BluetoothDeviceController::BluetoothDeviceController(const std::string& address) noexcept:
    BluetoothDeviceController(address.c_str()){}
    
    BluetoothDeviceController::BluetoothDeviceController(const char* address) noexcept:
    _state(State::DEFAULT),
    _address(address){
        int res=bt_gatt_client_create(_address.c_str(), &_clientHandle);
        Logger::showResultError("bt_gatt_client_create id="+_address, res);
    }

    BluetoothDeviceController::~BluetoothDeviceController() noexcept{
        if(_state==State::CONNECTED) disconnect(); 
        int res=bt_gatt_client_destroy(_clientHandle);
        Logger::showResultError("bt_gatt_client_destroy", res);
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
            Logger::showResultError("bt_gatt_connect", res);

            _state=State::CONNECTING;

            cv.wait(lock, [&](){
                return _state!=State::CONNECTING;
            });
        }else if(_state==State::CONNECTED){
            Logger::log(LogLevel::WARNING, "already connected to device "+_address);
        }else{
            Logger::log(LogLevel::ERROR,  "connect aborted.");
        }
    }
    auto BluetoothDeviceController::disconnect() noexcept -> void {
        Logger::log(LogLevel::DEBUG, "explicit disconnect call");
        std::unique_lock lock(operationM);
        if(_state==State::CONNECTED || _state==State::CONNECTING){
            _state=State::DISCONNECTED;
            int res=bt_gatt_disconnect(_address.c_str());
            Logger::showResultError("bt_gatt_disconnect", res);
            
        }else{
            Logger::log(LogLevel::ERROR, "cannot disconnect. Device not connected "+_address);
        }
    }
    auto BluetoothDeviceController::connectionStateCallback(int res, bool connected, const char* remote_address, void* user_data) noexcept -> void{
        std::string err=get_error_message(res);
        Logger::log(LogLevel::DEBUG, "callback called for device "+std::string(remote_address)+" with state="+std::to_string(connected)+" and result="+err);
        BluetoothManager& bluetoothManager = *static_cast<BluetoothManager*> (user_data);
        std::scoped_lock lock(bluetoothManager.bluetoothDevices().mut);
        auto& device=bluetoothManager.bluetoothDevices().var[remote_address];
        std::scoped_lock devLock(device.operationM);
        if(!res){
            if(connected && device.cState()==State::CONNECTING){
                device.state()=State::CONNECTED;
                Logger::log(LogLevel::DEBUG, "connected to device "+device.cAddress());
            }else{
                Logger::log(LogLevel::DEBUG, "state="+std::to_string((int)device.cState()));
                device.state()=State::DISCONNECTED;
                Logger::log(LogLevel::DEBUG, "disconnected from device "+device.cAddress());
            }
        }else{
            Logger::log(LogLevel::ERROR, "connectionStateCallback " + err);
            device.state()=State::DISCONNECTED;
        }
        device.cv.notify_one();
    }
};