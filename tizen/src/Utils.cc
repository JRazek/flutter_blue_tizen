#include <Utils.h>
#include <Logger.h>
#include <GATT/BluetoothService.h>

namespace btu{
    using namespace btlog;

    auto localToProtoDeviceState(const BluetoothDeviceController::State& s) -> proto::gen::DeviceStateResponse_BluetoothDeviceState{
        using State=btu::BluetoothDeviceController::State;
        switch (s){
            case State::CONNECTED: return proto::gen::DeviceStateResponse_BluetoothDeviceState_CONNECTED;
            case State::CONNECTING: return proto::gen::DeviceStateResponse_BluetoothDeviceState_CONNECTING;
            case State::DISCONNECTED: return proto::gen::DeviceStateResponse_BluetoothDeviceState_DISCONNECTED;
            case State::DISCONNECTING: return proto::gen::DeviceStateResponse_BluetoothDeviceState_DISCONNECTING;
            default: return proto::gen::DeviceStateResponse_BluetoothDeviceState_DISCONNECTED;
        }
    }

    auto messageToVector(const google::protobuf::MessageLite& messageLite) noexcept -> std::vector<u_int8_t>{
        std::vector<u_int8_t> encoded(messageLite.ByteSizeLong());
        messageLite.SerializeToArray(encoded.data(), messageLite.ByteSizeLong());
        return encoded;
    }

    auto getProtoServices(bt_gatt_client_h handle) -> std::vector<proto::gen::BluetoothService> {
        std::vector<proto::gen::BluetoothService> services;

        int res=bt_gatt_client_foreach_services(handle, &serviceForeachCallback, &services);
        Logger::showResultError("bt_gatt_client_foreach_services", res);
        std::string remoteID;

        if(!res){
            for(auto& service : services){
                service.set_remote_id(getGattClientAddress(handle).c_str());

                auto includedServices=getProtoIncludedServices(getGattService(handle, service.uuid()));
                for(auto& is : includedServices){
                    *service.add_included_services()=std::move(is);
                }

                for(int i=0;i<service.characteristics_size();i++){
                    auto& characteristic=*service.mutable_characteristics(i);
                    characteristic.set_remote_id(service.remote_id());
                    if(!service.is_primary()){
                        characteristic.set_secondaryserviceuuid(service.uuid());
                    }
                    characteristic.set_serviceuuid(service.uuid());
                    Logger::log(LogLevel::DEBUG, "set_serviceuuid.set_uuid - "+characteristic.serviceuuid());

                    for(int j=0;j<characteristic.descriptors_size(); j++){
                        auto& descriptor=*characteristic.mutable_descriptors(j);

                        descriptor.set_remote_id(service.remote_id());

                        descriptor.set_characteristicuuid(characteristic.uuid());
                        descriptor.set_serviceuuid(service.uuid());
                    }
                }
            }
        }

        return services;
    }
    auto getProtoIncludedServices(bt_gatt_h service_handle) -> std::vector<proto::gen::BluetoothService> {
        std::vector<proto::gen::BluetoothService> services;

        Logger::log(LogLevel::DEBUG, "debug10");
        int res=bt_gatt_service_foreach_included_services(service_handle, &serviceForeachCallback, &services);
        Logger::showResultError("bt_gatt_service_foreach_included_services", res);
        Logger::log(LogLevel::DEBUG, "debug11");

        return services;
    }

    auto getProtoCharacteristics(bt_gatt_h service_handle) -> std::vector<proto::gen::BluetoothCharacteristic> {
        std::vector<proto::gen::BluetoothCharacteristic> characteristics;
        int res=bt_gatt_service_foreach_characteristics(service_handle, 
        [](int total, int index, bt_gatt_h characteristic_handle, void* user_data) -> bool {
            auto& characteristics=*static_cast<std::vector<proto::gen::BluetoothCharacteristic> *>(user_data);
            auto& characteristic=characteristics.emplace_back();

            characteristic.set_uuid(getGattUUID(characteristic_handle));
            characteristic.set_value(getGattValue(characteristic_handle));

            auto descriptors=getProtoDescriptors(characteristic_handle);

            for(auto& d : descriptors){
                d.set_characteristicuuid(characteristic.uuid());
                *characteristic.add_descriptors()=std::move(d);
            }

            // characteristic.set_allocated_properties(new proto::gen::CharacteristicProperties(getProtoCharacteristicProperties(characteristic_handle)));

            return true;
        }, &characteristics);
        Logger::showResultError("bt_gatt_service_foreach_characteristics", res);

        return characteristics;
    }

    auto getProtoDescriptors(bt_gatt_h characteristic_handle) -> std::vector<proto::gen::BluetoothDescriptor> {
        std::vector<proto::gen::BluetoothDescriptor> descriptors;
        int res=bt_gatt_characteristic_foreach_descriptors(characteristic_handle,
        [] (int total, int index, bt_gatt_h descriptor_handle, void* user_data) -> bool {
            auto& descriptors=*static_cast<std::vector<proto::gen::BluetoothDescriptor> *>(user_data);
            auto& descriptor=descriptors.emplace_back();

            descriptor.set_uuid(getGattUUID(descriptor_handle));
            descriptor.set_value(getGattValue(descriptor_handle));

            return true;
            
        }, &descriptors);
        Logger::showResultError("bt_gatt_characteristic_foreach_descriptors", res);

        return descriptors;
    }

    auto getProtoCharacteristicProperties(int properties) -> proto::gen::CharacteristicProperties {
        proto::gen::CharacteristicProperties p;
        p.set_broadcast((properties & 1) != 0);
        p.set_read((properties & 2) != 0);
        p.set_write_without_response((properties & 4) != 0);
        p.set_write((properties & 8) != 0);
        p.set_notify((properties & 16) != 0);
        p.set_indicate((properties & 32) != 0);
        p.set_authenticated_signed_writes((properties & 64) != 0);
        p.set_extended_properties((properties & 128) != 0);
        p.set_notify_encryption_required((properties & 256) != 0);
        p.set_indicate_encryption_required((properties & 512) != 0);
        return p;
    }

    /**
     * @brief Get the value of Gatt descriptor, characteristic
     * 
     * @param handle 
     * @return std::string 
     */
    auto getGattValue(bt_gatt_h handle) -> std::string {
        std::string result="";
        char* value=nullptr;
        int length=0;

        int res=bt_gatt_get_value(handle, &value, &length);
        Logger::showResultError("bt_gatt_get_value", res);
        if(!res && value){
            result=std::string(value, length);
            free(value);
        }

        return result;
    }

    /**
     * @brief Get the uuid of Gatt descriptor, characteristic or service
     * 
     * @param handle 
     * @return std::string 
     */
    auto getGattUUID(bt_gatt_h handle) -> std::string {
        std::string result;
        char* uuid=nullptr;
        int res=bt_gatt_get_uuid(handle, &uuid);
        Logger::showResultError("bt_gatt_get_uuid", res);
        if(!res && uuid){
            result=std::string(uuid);
            free(uuid);
        }
        return result;
    }

    auto getGattClientAddress(bt_gatt_client_h handle) -> std::string {
        std::string result;
        char* address=nullptr;
        int res=bt_gatt_client_get_remote_address(handle, &address);
        Logger::showResultError("bt_gatt_client_get_remote_address", res);
        if(!res && address){
            result=std::string(address);
            free(address);
        }
        return result;        
    }
    auto getProtoServiceDiscoveryResult(const BluetoothDeviceController& device, const std::vector<std::shared_ptr<btGatt::PrimaryService>>& services) -> proto::gen::DiscoverServicesResult {
        proto::gen::DiscoverServicesResult res;
        Logger::log(LogLevel::DEBUG, "in order");
        for(const auto& s : services){
            *res.add_services() = s->toProtoService();
        }
        res.set_remote_id(device.cAddress());
        Logger::log(LogLevel::DEBUG, "out order");
        return res;
    }



    auto getGattService(bt_gatt_client_h handle, const std::string& uuid) -> bt_gatt_h {
        bt_gatt_h result;
        int res=bt_gatt_client_get_service(handle, uuid.c_str(), &result);
        Logger::showResultError("bt_gatt_client_get_service", res);
        
        return result;
    }

    auto serviceForeachCallback(int total, int index, bt_gatt_h service_handle, void* user_data) -> bool {
        auto& services=*static_cast<std::vector<proto::gen::BluetoothService> *>(user_data);

        proto::gen::BluetoothService& service=services.emplace_back();
        
        service.set_uuid(getGattUUID(service_handle));
        service.set_is_primary(false);

        auto characteristics=getProtoCharacteristics(service_handle);

        for(auto& c : characteristics){
            *service.add_characteristics()=std::move(c);
        }
    
        return true;
    }
}