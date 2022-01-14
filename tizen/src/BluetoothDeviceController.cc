#include <BluetoothDeviceController.h>
#include <Logger.h>
#include <BluetoothManager.h>
#include <NotificationsHandler.h>
#include <Utils.h>

#include <mutex>
#include <unordered_set>
namespace btu {
    using namespace btlog;

    BluetoothDeviceController::BluetoothDeviceController(const std::string& address, NotificationsHandler& notificationsHandler) noexcept:
    BluetoothDeviceController(address.c_str(), notificationsHandler){}
    
    BluetoothDeviceController::BluetoothDeviceController(const char* address, NotificationsHandler& notificationsHandler) noexcept:
    _address(address),
    _notificationsHandler(notificationsHandler)
    {}

    BluetoothDeviceController::~BluetoothDeviceController() noexcept{
        if(state()==State::CONNECTED) disconnect();
        Logger::log(LogLevel::DEBUG, "reporting destroy!");
    }

    auto BluetoothDeviceController::cAddress() const noexcept -> const decltype(_address)& { return _address; }
    auto BluetoothDeviceController::state() noexcept -> State {
        bool isConnected;
        
        int res=bt_device_is_profile_connected(_address.c_str(), BT_PROFILE_GATT, &isConnected);
        Logger::showResultError("bt_device_is_profile_connected", res);
        return (isConnected ? State::CONNECTED : State::DISCONNECTED); 
    }
    // auto BluetoothDeviceController::cState() const noexcept -> const decltype(_state)& { return _state; }
    auto BluetoothDeviceController::protoBluetoothDevices() noexcept -> decltype(_protoBluetoothDevices)& { return _protoBluetoothDevices; }
    auto BluetoothDeviceController::cProtoBluetoothDevices() const noexcept -> const decltype(_protoBluetoothDevices)& { return _protoBluetoothDevices; }

    auto BluetoothDeviceController::connect(const ConnectRequest& connReq) noexcept -> void {
        using namespace std::literals;
        std::unique_lock lock(operationM);
        if(state()==State::DISCONNECTED){
            int res=bt_gatt_connect(_address.c_str(), false);
            Logger::showResultError("bt_gatt_connect", res);
        }else{
            Logger::log(LogLevel::WARNING, "already connected to device "+_address);
        }
    }
    auto BluetoothDeviceController::disconnect() noexcept -> void {
        Logger::log(LogLevel::DEBUG, "explicit disconnect call");
        std::unique_lock lock(operationM);
        if(state()==State::CONNECTED){
            int res=bt_gatt_disconnect(_address.c_str());
            Logger::showResultError("bt_gatt_disconnect", res);
        }else{
            Logger::log(LogLevel::WARNING, "cannot disconnect. Device not connected "+_address);
        }
    }
    auto BluetoothDeviceController::connectionStateCallback(int res, bool connected, const char* remote_address, void* user_data) noexcept -> void{
        std::string err=get_error_message(res);
        Logger::log(LogLevel::DEBUG, "callback called for device "+std::string(remote_address)+" with state="+std::to_string(connected)+" and result="+err);
        
        if(!res){
            BluetoothManager& bluetoothManager = *static_cast<BluetoothManager*> (user_data);
            std::scoped_lock lock(bluetoothManager.bluetoothDevices().mut);
            auto ptr=bluetoothManager.bluetoothDevices().var.find(remote_address);
            //when disconnect is called from destructor, this callback can be invoked when the object is already destroyed.
            if(ptr!=bluetoothManager.bluetoothDevices().var.end()){
                auto device=(*ptr).second;
                std::scoped_lock devLock(device->operationM);

                DeviceStateResponse devState;
                devState.set_remote_id(device->cAddress());
                devState.set_state(localToProtoDeviceState(device->state()));
                device->_notificationsHandler.notifyUIThread("DeviceState", devState);
            }
        }
    }
};