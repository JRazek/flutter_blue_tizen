#include <GATT/BluetoothCharacteristic.h>
#include <GATT/BluetoothDescriptor.h>
#include <GATT/BluetoothService.h>
#include <BluetoothDeviceController.h>
#include <Utils.h>
#include <Logger.h>
#include <NotificationsHandler.h>

namespace btGatt{
    using namespace btu;
    using namespace btlog;
    BluetoothCharacteristic::BluetoothCharacteristic(bt_gatt_h handle, BluetoothService& service):
    _handle(handle),
    _service(service){
        int res=bt_gatt_characteristic_get_properties(handle, &_properties);
        Logger::showResultError("bt_gatt_characteristic_get_properties", res);

        res=bt_gatt_characteristic_foreach_descriptors(handle, [](int total, int index, bt_gatt_h descriptor_handle, void* scope_ptr) -> bool {
            auto& characteristic=*static_cast<BluetoothCharacteristic*>(scope_ptr);
            characteristic._descriptors.emplace_back(std::make_shared<BluetoothDescriptor>(descriptor_handle, characteristic));
            return true;
        }, this);
    }

    auto BluetoothCharacteristic::toProtoCharacteristic() const noexcept -> proto::gen::BluetoothCharacteristic{
        proto::gen::BluetoothCharacteristic proto;
        proto.set_remote_id(_service.cDevice().cAddress());
        proto.set_uuid(UUID());
        proto.set_allocated_properties(new proto::gen::CharacteristicProperties(getProtoCharacteristicProperties(_properties)));
        proto.set_value(value());
        if(_service.getType()==ServiceType::PRIMARY)
            proto.set_serviceuuid(_service.UUID());
        else{
            SecondaryService& sec=dynamic_cast<SecondaryService&>(_service);
            proto.set_serviceuuid(sec.UUID());
            proto.set_secondaryserviceuuid(sec.primaryUUID());
        }
        for(const auto& descriptor:_descriptors){
            *proto.add_descriptors()=descriptor->toProtoDescriptor();
        }
        return proto;
    }
    auto BluetoothCharacteristic::cService() const noexcept -> const decltype(_service)& {
        return _service;
    }
    auto BluetoothCharacteristic::UUID() const noexcept -> std::string {
        return getGattUUID(_handle);
    }
    auto BluetoothCharacteristic::value() const noexcept -> std::string {
        return (_valueFetched ? getGattValue(_handle):"");
    }
    auto BluetoothCharacteristic::getDescriptor(const std::string& uuid) -> std::shared_ptr<BluetoothDescriptor> {
        for(auto s: _descriptors){
            if(s->UUID()==uuid)
                return s;
        }
        return {};
    }
    auto BluetoothCharacteristic::read(const std::function<void(BluetoothCharacteristic& characteristic)>& func) -> void {
        struct Scope{
            std::function<void(BluetoothCharacteristic& characteristic)> func;
            BluetoothCharacteristic& characteristic;
        };
        Scope* scope=new Scope{func, *this};//unfortunately it requires raw ptr
        int res=bt_gatt_client_read_value(_handle, 
            [](int result, bt_gatt_h request_handle, void* scope_ptr){
                if(!result){
                    Logger::log(LogLevel::DEBUG, "result Success");
                    Scope& scope=*static_cast<Scope*>(scope_ptr);
                    Logger::log(LogLevel::DEBUG, "passed static_cast!!!");
                    auto& characteristic=scope.characteristic;
                    characteristic._valueFetched=true;
                    Logger::log(LogLevel::DEBUG, "calling cb...");
                    scope.func(characteristic);
                }else{
                    Logger::showResultError("bt_gatt_client_request_completed_cb", result);
                }
                delete scope_ptr;
        }, scope);
        Logger::showResultError("bt_gatt_client_read_value", res);
    }

}