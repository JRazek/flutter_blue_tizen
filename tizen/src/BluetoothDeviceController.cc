#include <BluetoothDeviceController.h>
#include <Logger.h>
#include <BluetoothManager.h>
#include <NotificationsHandler.h>
#include <Utils.h>

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
        disconnect();
        _services.clear();
        destroyGattClientIfExists(_address);
        Logger::log(LogLevel::DEBUG, "reporting destroy!");
    }

    auto BluetoothDeviceController::cAddress() const noexcept -> const decltype(_address)& { return _address; }
    auto BluetoothDeviceController::state() const noexcept -> State {
        if(isConnecting^isDisconnecting){
            return (isConnecting ? State::CONNECTING : State::DISCONNECTING);
        }else{
            bool isConnected=false;
            int res=bt_device_is_profile_connected(_address.c_str(), BT_PROFILE_GATT, &isConnected);
            Logger::showResultError("bt_device_is_profile_connected", res);
            return (isConnected ? State::CONNECTED : State::DISCONNECTED); 
        }
    }
    auto BluetoothDeviceController::protoBluetoothDevices() noexcept -> decltype(_protoBluetoothDevices)& { return _protoBluetoothDevices; }
    auto BluetoothDeviceController::cProtoBluetoothDevices() const noexcept -> const decltype(_protoBluetoothDevices)& { return _protoBluetoothDevices; }
    auto BluetoothDeviceController::connect(const proto::gen::ConnectRequest& connReq) noexcept -> void {
        std::unique_lock lock(operationM);
        if(state()==State::DISCONNECTED){
            isConnecting=true;
            int res=bt_gatt_connect(_address.c_str(), connReq.android_auto_connect());
            Logger::showResultError("bt_gatt_connect", res);
        }else{
            Logger::log(LogLevel::WARNING, "already connected to device "+_address);
        }
    }
    auto BluetoothDeviceController::disconnect() noexcept -> void {
        std::lock_guard lock(operationM);

        if(state()==State::CONNECTED){
        Logger::log(LogLevel::DEBUG, "explicit disconnect call");
            isDisconnecting=true;
            int res=bt_gatt_disconnect(_address.c_str());
            Logger::showResultError("bt_gatt_disconnect", res);
        }else{
            // Logger::log(LogLevel::WARNING, "cannot disconnect. Device not connected "+_address);
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
            if((res==BT_ERROR_NONE || res==BT_ERROR_ALREADY_DONE) && client){
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

    auto BluetoothDeviceController::discoverServices() noexcept -> std::vector<btGatt::PrimaryService*> {
        std::scoped_lock lock(operationM);

        struct Scope{
            BluetoothDeviceController& device;
            std::vector<std::unique_ptr<btGatt::PrimaryService>>& services;
        };

        //unsafe block (void *)
        Scope scope{*this, _services};

        int res=bt_gatt_client_foreach_services(getGattClient(_address), [](int total, int index, bt_gatt_h service_handle, void* scope_ptr) -> bool {
            auto& scope=*static_cast<Scope*>(scope_ptr);

            scope.services.emplace_back(std::make_unique<btGatt::PrimaryService>(service_handle, scope.device));
            
            return true;
        }, &scope);
        ///////////////

        Logger::showResultError("bt_gatt_client_foreach_services", res);
        auto result=std::vector<btGatt::PrimaryService*>();
        for(auto& s:_services){
            result.emplace_back(s.get());
        }
        return result;
    }
    auto BluetoothDeviceController::getService(const std::string& uuid) noexcept -> PrimaryService*{ 
        for(auto& s: _services){
            if(s->UUID()==uuid)
                return s.get();
        }
        return nullptr;
    }
    auto BluetoothDeviceController::cNotificationsHandler() const noexcept -> const NotificationsHandler& {
        return _notificationsHandler;
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
                device->isConnecting=false;
                device->isDisconnecting=false;
                std::scoped_lock devLock(device->operationM);
                if(!connected){
                    device->_services.clear();
                    std::scoped_lock l(gatt_clients.mut);
                    gatt_clients.var.erase(device->cAddress());
                }

                proto::gen::DeviceStateResponse devState;
                devState.set_remote_id(device->cAddress());
                devState.set_state(localToProtoDeviceState(device->state()));
                device->_notificationsHandler.notifyUIThread("DeviceState", devState);
            }
        }
    }

};