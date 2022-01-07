#include <BluetoothDeviceController.h>
#include <Logger.h>

#include <mutex>
namespace btu {
    using namespace btlog;
    BluetoothDeviceController::BluetoothDeviceController() noexcept:
    _state(State::DEFAULT),
    gattServerHandle({nullptr, false}){}

    BluetoothDeviceController::~BluetoothDeviceController() noexcept{
        std::scoped_lock lock(gattServerHandle.mut);
        auto& handle=gattServerHandle.var.first;
        const auto& isConnected=gattServerHandle.var.second;

    }

    auto BluetoothDeviceController::address() noexcept -> decltype(_address)& { return _address; }
    auto BluetoothDeviceController::cAddress() const noexcept -> const decltype(_address)& { return _address; }
    auto BluetoothDeviceController::state() noexcept -> decltype(_state)& { return _state; }
    auto BluetoothDeviceController::cState() const noexcept -> const decltype(_state)&{ return _state; }
    auto BluetoothDeviceController::protoBluetoothDevices() noexcept -> decltype(_protoBluetoothDevices)&{ return _protoBluetoothDevices; }
    auto BluetoothDeviceController::cProtoBluetoothDevices() const noexcept -> const decltype(_protoBluetoothDevices)&{ return _protoBluetoothDevices; }

    auto BluetoothDeviceController::connect() -> void {
        std::scoped_lock lock(gattServerHandle.mut);
        auto& handle=gattServerHandle.var.first;
        auto& isConnected=gattServerHandle.var.second;
        if(!isConnected){
            int res=bt_gatt_server_create(&handle);
            isConnected=true;
        }else{
            Logger::log(LogLevel::ERROR, "already connected to device "+_address);
        }
    }
    auto BluetoothDeviceController::disconnect() -> void {
        std::scoped_lock lock(gattServerHandle.mut);
        auto& handle=gattServerHandle.var.first;
        auto& isConnected=gattServerHandle.var.second;
        if(isConnected){
            int res=bt_gatt_service_destroy(handle);
            isConnected=false;
            
        }else{
            Logger::log(LogLevel::ERROR, "cannot disconnect. Device not connected "+_address);
        }
    }

};