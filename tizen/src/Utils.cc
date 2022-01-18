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
            Logger::log(LogLevel::DEBUG, "debug0");

            auto& services=*static_cast<std::vector<BluetoothService> *>(user_data);
        
            Logger::log(LogLevel::DEBUG, "debug1");

            char* uuid=nullptr;
            int res=bt_gatt_get_uuid(service_handle, &uuid);
            Logger::showResultError("bt_gatt_get_uuid services", res);
            Logger::log(LogLevel::DEBUG, "debug2");
            
            if(!res && uuid){

                Logger::log(LogLevel::DEBUG, "debug3");
                BluetoothService& service=services.emplace_back();
                
                service.set_uuid(uuid);
                Logger::log(LogLevel::DEBUG, "debug4");
                free(uuid);     
                auto characteristics=getProtoCharacteristics(service_handle);
                for(auto& c : characteristics){
                    c.set_serviceuuid(service.uuid());
                    *service.add_characteristics()=c;
                }
            }
            return true;
        }, &services);
        return services;
    }


    auto getProtoCharacteristics(bt_gatt_h service_handle) -> std::vector<BluetoothCharacteristic> {
        std::vector<BluetoothCharacteristic> characteristics;
        int res=bt_gatt_service_foreach_characteristics(service_handle, 
        [](int total, int index, bt_gatt_h characteristic_handle, void* user_data) -> bool {
            auto& characteristics=*static_cast<std::vector<BluetoothCharacteristic> *>(user_data);
            auto& characteristic=characteristics.emplace_back();
            char* uuid=nullptr;
            int res=bt_gatt_get_uuid(characteristic_handle, &uuid);
            Logger::showResultError("bt_gatt_get_uuid characteristics", res);
            if(!res && uuid){
                characteristic.set_uuid(uuid);
                int properties;
                res=bt_gatt_characteristic_get_properties(characteristic_handle, &properties);
                Logger::showResultError("bt_gatt_characteristic_get_properties", res);

                // characteristic.set_allocated_properties()
                free(uuid);
            }

            // res=bt_gatt_characteristic_foreach_descriptors(characteristic_handle, []
            // (int total, int index, bt_gatt_h characteristic_handle, void* user_data) -> bool {

            // }, )

            return true;
        }, &characteristics);
        // Logger::showResultError("bt_gatt_service_foreach_included_services", res);

        return characteristics;
    }
}