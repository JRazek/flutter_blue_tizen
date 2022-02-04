#include <GATT/BluetoothDescriptor.h>
#include <GATT/BluetoothCharacteristic.h>
#include <BluetoothDeviceController.h>
#include <Utils.h>
#include <Logger.h>

namespace btGatt{
    using namespace btu;
    using namespace btlog;
    BluetoothDescriptor::BluetoothDescriptor(bt_gatt_h handle, BluetoothCharacteristic& characteristic):
    _handle(handle),
    _characteristic(characteristic){}
    
    auto BluetoothDescriptor::toProtoDescriptor() const noexcept -> proto::gen::BluetoothDescriptor{
        proto::gen::BluetoothDescriptor proto;
        proto.set_remote_id(_characteristic.cService().cDevice().cAddress());
        proto.set_serviceuuid(_characteristic.cService().UUID());
        proto.set_characteristicuuid(_characteristic.UUID());
        proto.set_uuid(UUID());
        return proto;
    }
    auto BluetoothDescriptor::UUID() const noexcept -> std::string {
        return getGattUUID(_handle);
    }
    auto BluetoothDescriptor::value() const noexcept -> std::string{
        return getGattValue(_handle);
    }
    auto BluetoothDescriptor::read(const std::function<void(const BluetoothDescriptor&)>& func) -> void{
        struct Scope{
            std::function<void(const BluetoothDescriptor&)> func;
            const BluetoothDescriptor& descriptor;
        };
        Scope* scope=new Scope{func, *this};//unfortunately it requires raw ptr
        int res=bt_gatt_client_read_value(_handle, 
            [](int result, bt_gatt_h request_handle, void* scope_ptr){
                if(!result){
                    Scope& scope=*static_cast<Scope*>(scope_ptr);
                    auto& descriptor=scope.descriptor;
                    scope.func(descriptor);
                }
                Logger::showResultError("bt_gatt_client_request_completed_cb", result);
                
                delete scope_ptr;
        }, scope);
        Logger::showResultError("bt_gatt_client_read_value", res);
        if(res) throw BTException("could not read descriptor"); 
    }
    auto BluetoothDescriptor::write(const std::string value, bool withResponse, const std::function<void(bool success, const BluetoothCharacteristic&)>& callback) -> void {
        throw BTException("could not write to descriptor");
    }
    auto BluetoothDescriptor::cCharacteristic() const noexcept -> const BluetoothCharacteristic& {
        return _characteristic;
    }
}