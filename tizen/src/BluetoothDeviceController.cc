#include <BluetoothDeviceController.h>
#include <Logger.h>
#include <BluetoothManager.h>
#include <NotificationsHandler.h>
#include <Utils.h>
#include <GATT/BluetoothService.h>

#include <mutex>
#include <unordered_set>
namespace btu {
    using namespace btlog;
    using namespace btGatt;

    BluetoothDeviceController::BluetoothDeviceController(const std::string& address, NotificationsHandler& notificationsHandler) noexcept:
    BluetoothDeviceController(address.c_str(), notificationsHandler){}
    
    BluetoothDeviceController::BluetoothDeviceController(const char* address, NotificationsHandler& notificationsHandler) noexcept:
    _address(address),
    _notificationsHandler(notificationsHandler)
    {}

    BluetoothDeviceController::~BluetoothDeviceController() noexcept {
        if(state()==State::CONNECTED) disconnect();
        destroyGattClientIfExists(_address);
        Logger::log(LogLevel::DEBUG, "reporting destroy!");
    }

    auto BluetoothDeviceController::cAddress() const noexcept -> const decltype(_address)& { return _address; }
    auto BluetoothDeviceController::state() noexcept -> State {
        bool isConnected;
        
        int res=bt_device_is_profile_connected(_address.c_str(), BT_PROFILE_GATT, &isConnected);
        Logger::showResultError("bt_device_is_profile_connected", res);
        return (isConnected ? State::CONNECTED : State::DISCONNECTED); 
    }
    auto BluetoothDeviceController::protoBluetoothDevices() noexcept -> decltype(_protoBluetoothDevices)& { return _protoBluetoothDevices; }
    auto BluetoothDeviceController::cProtoBluetoothDevices() const noexcept -> const decltype(_protoBluetoothDevices)& { return _protoBluetoothDevices; }
    auto BluetoothDeviceController::connect(const proto::gen::ConnectRequest& connReq) noexcept -> void {
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
    auto BluetoothDeviceController::connectionStateCallback(int res, bool connected, const char* remote_address, void* user_data) noexcept -> void {
        std::string err=get_error_message(res);
        Logger::log(LogLevel::DEBUG, "callback called for device "+std::string(remote_address)+" with state="+std::to_string(connected)+" and result="+err);
        
        if(!res){
            auto& bluetoothManager=*static_cast<BluetoothManager*> (user_data);
            std::scoped_lock lock(bluetoothManager.bluetoothDevices().mut);
            auto ptr=bluetoothManager.bluetoothDevices().var.find(remote_address);
            //when disconnect is called from destructor, this callback can be invoked when the object is already destroyed.
            if(ptr!=bluetoothManager.bluetoothDevices().var.end()){
                auto device=(*ptr).second;
                std::scoped_lock devLock(device->operationM);

                proto::gen::DeviceStateResponse devState;
                devState.set_remote_id(device->cAddress());
                devState.set_state(localToProtoDeviceState(device->state()));
                device->_notificationsHandler.notifyUIThread("DeviceState", devState);
            }
        }
    }

    namespace {
        static SafeType<std::unordered_map<std::string, bt_gatt_client_h>> gatt_clients;
    }
    auto BluetoothDeviceController::getGattClient(const std::string& address) noexcept -> bt_gatt_client_h {
        std::scoped_lock lock(gatt_clients.mut);
        auto it=gatt_clients.var.find(address);
        bt_gatt_client_h client=nullptr;

        if(it==gatt_clients.var.end()){
            int res=bt_gatt_client_create(address.c_str(), &client);
            Logger::showResultError("bt_gatt_client_create", res);
            if(res==BT_ERROR_NONE || res==BT_ERROR_ALREADY_DONE){
                gatt_clients.var.emplace(address, client);
                Logger::log(LogLevel::DEBUG, "creating new gatt client for "+address);
            };
        }else{
            client=it->second;   
            Logger::log(LogLevel::DEBUG, "gatt client already exists. Returning for "+address);
        }
        return client;
    }
    auto BluetoothDeviceController::destroyGattClientIfExists(const std::string& address) noexcept -> void {
        std::scoped_lock lock(gatt_clients.mut);
        auto it=gatt_clients.var.find(address);
        if(it!=gatt_clients.var.end()){
            Logger::log(LogLevel::DEBUG, "destroying gatt client for "+address);
            bt_gatt_client_destroy(it->second);
            gatt_clients.var.erase(address);
        }
    }
    //should return std::vector<btGatt::PrimaryService>
    auto BluetoothDeviceController::discoverServices() noexcept -> void {
        std::scoped_lock lock(operationM);
        // std::vector<btGatt::PrimaryService> services;
        int res=bt_gatt_client_foreach_services(getGattClient(_address), [](int total, int index, bt_gatt_h service_handle, void* scope_ptr) -> bool {
            auto& device=*static_cast<BluetoothDeviceController*>(scope_ptr);
            

            return true;
        }, this);
        Logger::showResultError("bt_gatt_client_foreach_services", res);
    }
};