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
        auto scope=new Scope{func, *this};//unfortunately it requires raw ptr
        int res=bt_gatt_client_read_value(_handle, 
            [](int result, bt_gatt_h request_handle, void* scope_ptr){
                auto scope=static_cast<Scope*>(scope_ptr);
                if(!result){
                    auto& descriptor=scope->descriptor;
                    scope->func(descriptor);
                }
                Logger::showResultError("bt_gatt_client_request_completed_cb", result);
                
                delete scope;
        }, scope);
        Logger::showResultError("bt_gatt_client_read_value", res);
        if(res) throw BTException("could not read descriptor"); 
    }
    auto BluetoothDescriptor::write(const std::string value, const std::function<void(bool success, const BluetoothDescriptor&)>& callback) -> void {
        struct Scope{
            std::function<void(bool success, const BluetoothDescriptor&)> func;
            const BluetoothDescriptor& descriptor;
        };  
        Logger::log(LogLevel::DEBUG, "setting descriptor to value="+value+", with size="+std::to_string(value.size()));
        int res=bt_gatt_set_value(_handle, value.c_str(), value.size());
        Logger::showResultError("bt_gatt_set_value", res);

        if(res) throw BTException("could not set value");

        auto scope=new Scope{callback, *this};//unfortunately it requires raw ptr
        Logger::log(LogLevel::DEBUG, "characteristic write cb native");

        res=bt_gatt_client_write_value(_handle,
        [](int result, bt_gatt_h request_handle, void* scope_ptr){
            Logger::showResultError("bt_gatt_client_request_completed_cb", result);
            Logger::log(LogLevel::DEBUG, "descriptor write cb native");

            auto scope=static_cast<Scope*>(scope_ptr);
            auto& descriptor=scope->descriptor;
            scope->func(!result, descriptor);
            
            delete scope;
        }, scope);
        Logger::showResultError("bt_gatt_client_write_value", res);

        if(res) throw BTException("could not write value to remote");
    }
    auto BluetoothDescriptor::cCharacteristic() const noexcept -> const BluetoothCharacteristic& {
        return _characteristic;
    }
}