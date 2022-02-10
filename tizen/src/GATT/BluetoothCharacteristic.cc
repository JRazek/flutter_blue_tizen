#include <GATT/BluetoothCharacteristic.h>
#include <GATT/BluetoothDescriptor.h>
#include <GATT/BluetoothService.h>
#include <BluetoothDeviceController.h>
#include <Utils.h>
#include <Logger.h>
#include <NotificationsHandler.h>
#include <exception>

namespace btGatt{
    using namespace btu;
    using namespace btlog;
    BluetoothCharacteristic::BluetoothCharacteristic(bt_gatt_h handle, BluetoothService& service) noexcept:
    _handle(handle),
    _service(service){
        int res=bt_gatt_characteristic_foreach_descriptors(handle, [](int total, int index, bt_gatt_h descriptor_handle, void* scope_ptr) -> bool {
            auto& characteristic=*static_cast<BluetoothCharacteristic*>(scope_ptr);
            characteristic._descriptors.emplace_back(std::make_unique<BluetoothDescriptor>(descriptor_handle, characteristic));
            return true;
        }, this);
    }

    auto BluetoothCharacteristic::toProtoCharacteristic() const noexcept -> proto::gen::BluetoothCharacteristic{
        proto::gen::BluetoothCharacteristic proto;
        proto.set_remote_id(_service.cDevice().cAddress());
        proto.set_uuid(UUID());
        proto.set_allocated_properties(new proto::gen::CharacteristicProperties(getProtoCharacteristicProperties(properties())));
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
        return getGattValue(_handle);
    }
    auto BluetoothCharacteristic::getDescriptor(const std::string& uuid) -> BluetoothDescriptor* {
        for(auto& s: _descriptors){
            if(s->UUID()==uuid)
                return s.get();
        }
        return nullptr;
    }
    auto BluetoothCharacteristic::read(const std::function<void(const BluetoothCharacteristic&)>& func) -> void {
        struct Scope{
            std::function<void(const BluetoothCharacteristic&)> func;
            const BluetoothCharacteristic& characteristic;
        };
        Scope* scope=new Scope{func, *this};//unfortunately it requires raw ptr
        int res=bt_gatt_client_read_value(_handle, 
            [](int result, bt_gatt_h request_handle, void* scope_ptr){
                auto scope=static_cast<Scope*>(scope_ptr);
                auto& characteristic=scope->characteristic;
                scope->func(characteristic);
                Logger::showResultError("bt_gatt_client_request_completed_cb", result);
                
                delete scope;
        }, scope);

        if(res) throw BTException("could not read client");
        
        Logger::showResultError("bt_gatt_client_read_value", res);
    }

    auto BluetoothCharacteristic::write(const std::string value, bool withoutResponse, const std::function<void(bool success, const BluetoothCharacteristic&)>& callback) -> void {
        struct Scope{
            std::function<void(bool success, const BluetoothCharacteristic&)> func;
            const BluetoothCharacteristic& characteristic;
        };  
        Logger::log(LogLevel::DEBUG, "setting characteristic to value="+value+", with size="+std::to_string(value.size()));

        int res=bt_gatt_characteristic_set_write_type(_handle, (withoutResponse ? BT_GATT_WRITE_TYPE_WRITE_NO_RESPONSE:BT_GATT_WRITE_TYPE_WRITE));

        if(res) throw BTException("could not set write type");

        res=bt_gatt_set_value(_handle, value.c_str(), value.size());
        Logger::showResultError("bt_gatt_set_value", res);

        if(res) throw BTException("could not set value");


        Scope* scope=new Scope{callback, *this};//unfortunately it requires raw ptr
        Logger::log(LogLevel::DEBUG, "characteristic write cb native");

        res=bt_gatt_client_write_value(_handle,
        [](int result, bt_gatt_h request_handle, void* scope_ptr){
            Logger::showResultError("bt_gatt_client_request_completed_cb", result);
            Logger::log(LogLevel::DEBUG, "characteristic write cb native");

            auto scope=static_cast<Scope*>(scope_ptr);
            auto& characteristic=scope->characteristic;
            scope->func(!result, characteristic);
            
            delete scope;
        }, scope);
        Logger::showResultError("bt_gatt_client_write_value", res);

        if(res) throw BTException("could not write value to remote");
    }

    auto BluetoothCharacteristic::properties() const noexcept -> int {
        auto prop=0;
        auto res=bt_gatt_characteristic_get_properties(_handle, &prop);
        Logger::showResultError("bt_gatt_characteristic_get_properties", res);
        return prop;
    }
    
    auto BluetoothCharacteristic::setNotifyCallback(const NotifyCallback& callback) -> void {
        auto p=properties();
        if(!(p & 0x30))
            throw BTException("cannot set callback! notify=0 && indicate=0");
        
        unsetNotifyCallback();

        _notifyCallback=std::make_unique<NotifyCallback>(callback);
        auto res=bt_gatt_client_set_characteristic_value_changed_cb(_handle,
        [](bt_gatt_h ch_handle, char* value, int len, void* scope_ptr){
            auto& characteristic=*static_cast<BluetoothCharacteristic*>(scope_ptr);

            characteristic._notifyCallback->operator()(characteristic);

        }, this);
    }
    
    void BluetoothCharacteristic::unsetNotifyCallback() {
        if(cService().cDevice().state()==btu::BluetoothDeviceController::State::CONNECTED && _notifyCallback){
            Logger::log(LogLevel::DEBUG, "unsubscribing from characteristic notifications...");
            auto res=bt_gatt_client_unset_characteristic_value_changed_cb(_handle);
            Logger::showResultError("bt_gatt_client_unset_characteristic_value_changed_cb", res);
        }
        _notifyCallback=nullptr;
    }
    BluetoothCharacteristic::~BluetoothCharacteristic() noexcept {
        unsetNotifyCallback();
        _descriptors.clear();
    }
}