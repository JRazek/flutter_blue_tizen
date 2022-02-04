#ifndef UTILS_H
#define UTILS_H

#include <flutter/method_channel.h>
#include <flutter/encodable_value.h>
#include <flutterblue.pb.h>
#include <BluetoothDeviceController.h>

#include <mutex>
#include <exception>
namespace btu{

    using MethodChannel = flutter::MethodChannel<flutter::EncodableValue>;

    template<typename T>
    struct SafeType{
        T var;
        std::mutex mut;

        SafeType(const T& t):var(t){}
        SafeType(T&& t):var(std::move(t)){}
        SafeType():var(T()){}
    };
    class BTException : public std::exception{
        std::string _m;
    public:
        BTException(const std::string& m):_m(m){}
        auto what() const noexcept -> const char* override{
            return _m.c_str();
        };
    };
    
    auto localToProtoDeviceState(const BluetoothDeviceController::State& s) -> proto::gen::DeviceStateResponse_BluetoothDeviceState;
    
    auto messageToVector(const google::protobuf::MessageLite& messageLite) noexcept -> std::vector<u_int8_t>;


    auto getGattValue(bt_gatt_h handle) -> std::string;
    auto getGattUUID(bt_gatt_h handle) -> std::string;
    auto getGattService(bt_gatt_client_h handle, const std::string& uuid) -> bt_gatt_h;
    auto getGattClientAddress(bt_gatt_client_h handle) -> std::string;
    auto getProtoServiceDiscoveryResult(const BluetoothDeviceController& device, const std::vector<std::shared_ptr<btGatt::PrimaryService>>& services) -> proto::gen::DiscoverServicesResult;

    /*
     * do not use the functions below
     */
    auto getProtoServices(bt_gatt_client_h handle) -> std::vector<proto::gen::BluetoothService>;
    
    auto getProtoIncludedServices(bt_gatt_h service_handle) -> std::vector<proto::gen::BluetoothService>;
    auto getProtoCharacteristics(bt_gatt_h service_handle) -> std::vector<proto::gen::BluetoothCharacteristic>;
    auto getProtoCharacteristicProperties(int properties) -> proto::gen::CharacteristicProperties;
    auto getProtoDescriptors(bt_gatt_h characteristic_handle) -> std::vector<proto::gen::BluetoothDescriptor>;
    auto serviceForeachCallback(int total, int index, bt_gatt_h service_handle, void* user_data) -> bool;
}
#endif //UTILS_H