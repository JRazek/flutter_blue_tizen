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

    auto getProtoCharacteristicProperties(int properties) -> proto::gen::CharacteristicProperties {
        proto::gen::CharacteristicProperties p;
        p.set_broadcast((properties & 0x01) != 0);
        p.set_read((properties &  0x02) != 0);
        p.set_write_without_response((properties & 0x04) != 0);
        p.set_write((properties & 0x08) != 0);
        p.set_notify((properties & 0x10) != 0);
        p.set_indicate((properties & 0x20) != 0);
        p.set_authenticated_signed_writes((properties & 0x40) != 0);
        p.set_extended_properties((properties & 0x80) != 0);
        // p.set_notify_encryption_required((properties & 256) != 0);
        // p.set_indicate_encryption_required((properties & 512) != 0);
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
    auto getProtoServiceDiscoveryResult(const BluetoothDeviceController& device, const std::vector<btGatt::PrimaryService*>& services) -> proto::gen::DiscoverServicesResult {
        proto::gen::DiscoverServicesResult res;
        for(const auto& s : services){
            *res.add_services() = s->toProtoService();
        }
        res.set_remote_id(device.cAddress());
        return res;
    }

    auto getGattService(bt_gatt_client_h handle, const std::string& uuid) -> bt_gatt_h {
        bt_gatt_h result;
        int res=bt_gatt_client_get_service(handle, uuid.c_str(), &result);
        Logger::showResultError("bt_gatt_client_get_service", res);
        
        return result;
    }
}