#include <Utils.h>
#include <Logger.h>

namespace btu{
    using namespace btlog;

    auto localToProtoDeviceState(const BluetoothDeviceController::State& s) -> DeviceStateResponse_BluetoothDeviceState{
        using State=btu::BluetoothDeviceController::State;
        switch (s){
            case State::CONNECTED: return DeviceStateResponse_BluetoothDeviceState_CONNECTED;
            // case State::CONNECTING: return DeviceStateResponse_BluetoothDeviceState_CONNECTING;
            case State::DISCONNECTED: return DeviceStateResponse_BluetoothDeviceState_DISCONNECTED;
            // case State::DISCONNECTING: return DeviceStateResponse_BluetoothDeviceState_DISCONNECTING;
            default: return DeviceStateResponse_BluetoothDeviceState_DISCONNECTED;
        }
    }

    auto messageToVector(const google::protobuf::MessageLite& messageLite) noexcept -> std::vector<u_int8_t>{
        std::vector<u_int8_t> encoded(messageLite.ByteSizeLong());
        messageLite.SerializeToArray(encoded.data(), messageLite.ByteSizeLong());
        return encoded;
    }

    auto getProtoServices(bt_gatt_client_h handle) -> std::vector<BluetoothService> {
        std::vector<BluetoothService> services;

        int res=bt_gatt_client_foreach_services(handle, 
        [](int total, int index, bt_gatt_h service_handle, void* user_data) -> bool {
            auto& services=*static_cast<std::vector<BluetoothService> *>(user_data);

            Logger::log(LogLevel::DEBUG, "debug3");
            BluetoothService& service=services.emplace_back();
            
            service.set_uuid(getGattUUID(service_handle));
            service.set_is_primary(false);
            Logger::log(LogLevel::DEBUG, "debug4");

            auto characteristics=getProtoCharacteristics(service_handle);

            for(auto& c : characteristics){
                *service.add_characteristics()=std::move(c);
            }
        
            return true;
        }, &services);

        std::string remoteID;

        for(auto& service : services){
            service.set_remote_id(getGattClientAddress(handle).c_str());
            
            for(int i=0;i<service.characteristics_size();i++){
                auto& characteristic=*service.mutable_characteristics(i);
                characteristic.set_remote_id(service.remote_id());
                if(!service.is_primary()){
                    characteristic.set_secondaryserviceuuid(service.uuid());
                }
                characteristic.set_serviceuuid(service.uuid());
                Logger::log(LogLevel::DEBUG, "set_serviceuuid.set_uuid - "+characteristic.serviceuuid());
                characteristic.set_value("test1");

                for(int j=0;j<characteristic.descriptors_size(); j++){
                    auto& descriptor=*characteristic.mutable_descriptors(j);

                    descriptor.set_remote_id(service.remote_id());

                    descriptor.set_characteristicuuid(characteristic.uuid());
                    descriptor.set_serviceuuid(service.uuid());
                    descriptor.set_value("test2");
                }
            }
        }

        return services;
    }


    auto getProtoCharacteristics(bt_gatt_h service_handle) -> std::vector<BluetoothCharacteristic> {
        std::vector<BluetoothCharacteristic> characteristics;
        int res=bt_gatt_service_foreach_characteristics(service_handle, 
        [](int total, int index, bt_gatt_h characteristic_handle, void* user_data) -> bool {
            auto& characteristics=*static_cast<std::vector<BluetoothCharacteristic> *>(user_data);
            auto& characteristic=characteristics.emplace_back();

            characteristic.set_uuid(getGattUUID(characteristic_handle));
            
            Logger::log(LogLevel::DEBUG, "characteristic.set_uuid - "+characteristic.uuid());

            auto descriptors=getProtoDescriptors(characteristic_handle);

            for(auto& d : descriptors){
                d.set_characteristicuuid(characteristic.uuid());
                *characteristic.add_descriptors()=std::move(d);
            }

            characteristic.set_allocated_properties(new CharacteristicProperties(getProtoCharacteristicProperties(characteristic_handle)));
            
            Logger::log(LogLevel::DEBUG, "debug6");
            

            return true;
        }, &characteristics);

        return characteristics;
    }

    auto getProtoDescriptors(bt_gatt_h characteristic_handle) -> std::vector<BluetoothDescriptor> {
        std::vector<BluetoothDescriptor> descriptors;
        int res=bt_gatt_characteristic_foreach_descriptors(characteristic_handle,
        [] (int total, int index, bt_gatt_h descriptor_handle, void* user_data) -> bool {
            auto& descriptors=*static_cast<std::vector<BluetoothDescriptor> *>(user_data);
            auto& descriptor=descriptors.emplace_back();

            descriptor.set_uuid(getGattUUID(descriptor_handle));
            descriptor.set_value(getGattValue(descriptor_handle));

            Logger::log(LogLevel::DEBUG, "debug7");
            return true;
            
        }, &descriptors);
        Logger::showResultError("bt_gatt_characteristic_foreach_descriptors", res);

        return descriptors;
    }

    auto getProtoCharacteristicProperties(bt_gatt_h characteristic_handle) -> CharacteristicProperties {
        CharacteristicProperties p;
        int properties=0;
        int res=bt_gatt_characteristic_get_properties(characteristic_handle, &properties);
        Logger::showResultError("bt_gatt_characteristic_get_properties", res);
        if(!res){
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
        }
        return p;
    }

    /**
     * @brief Get the value of Gatt descriptor, characteristic or service
     * 
     * @param handle 
     * @return std::string 
     */
    auto getGattValue(bt_gatt_h handle) -> std::string {
        std::string result="";
        char* value=nullptr;
        int length=0;

        int res=bt_gatt_get_value(handle, &value, &length);
        Logger::showResultError("bt_gatt_characteristic_get_properties", res);
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
}