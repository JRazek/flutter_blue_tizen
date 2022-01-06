#include <BluetoothDeviceController.h>

#include <mutex>
namespace btu {
    auto BluetoothDeviceController::address() noexcept -> decltype(_address)& { return _address; }
    auto BluetoothDeviceController::cAddress() const noexcept -> const decltype(_address)& { return _address; }
    auto BluetoothDeviceController::state() noexcept -> decltype(_state)& { return _state; }
    auto BluetoothDeviceController::cState() const noexcept -> const decltype(_state)&{ return _state; }
    auto BluetoothDeviceController::protoBluetoothDevices() noexcept -> decltype(_protoBluetoothDevices)&{ return _protoBluetoothDevices; }
    auto BluetoothDeviceController::cProtoBluetoothDevices() const noexcept -> const decltype(_protoBluetoothDevices)&{ return _protoBluetoothDevices; }

    auto BluetoothDeviceController::connect() -> void{
        std::scoped_lock lock(gattHandle.mut);
        auto& handle=gattHandle.var;

    }

};