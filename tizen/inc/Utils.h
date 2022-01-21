#ifndef UTILS_H
#define UTILS_H

#include <flutter/method_channel.h>
#include <flutter/encodable_value.h>
#include <flutterblue.pb.h>
#include <BluetoothDeviceController.h>

#include <mutex>
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
    
    auto localToProtoDeviceState(const BluetoothDeviceController::State& s) -> DeviceStateResponse_BluetoothDeviceState;
    
    auto messageToVector(const google::protobuf::MessageLite& messageLite) noexcept -> std::vector<u_int8_t>;

    auto getProtoServices(bt_gatt_client_h handle) -> std::vector<BluetoothService>;
    
    auto getProtoCharacteristics(bt_gatt_h service_handle) -> std::vector<BluetoothCharacteristic>;
    auto getProtoCharacteristicProperties(bt_gatt_h characteristic_handle) -> CharacteristicProperties;
    auto getProtoDescriptors(bt_gatt_h characteristic_handle) -> std::vector<BluetoothDescriptor>;
    auto getGattValue(bt_gatt_h handle) -> std::string;
    auto getGattUUID(bt_gatt_h handle) -> std::string;
    auto getGattClientAddress(bt_gatt_client_h handle) -> std::string;
}
#endif