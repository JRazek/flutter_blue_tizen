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
    static auto localToProtoDeviceState(const BluetoothDeviceController::State& s) -> DeviceStateResponse_BluetoothDeviceState{
        using State=btu::BluetoothDeviceController::State;
        switch (s){
            case State::CONNECTED: return DeviceStateResponse_BluetoothDeviceState_CONNECTED;
            // case State::CONNECTING: return DeviceStateResponse_BluetoothDeviceState_CONNECTING;
            case State::DISCONNECTED: return DeviceStateResponse_BluetoothDeviceState_DISCONNECTED;
            // case State::DISCONNECTING: return DeviceStateResponse_BluetoothDeviceState_DISCONNECTING;
            default: return DeviceStateResponse_BluetoothDeviceState_DISCONNECTED;
        }
    }
    static auto messageToVector(const google::protobuf::MessageLite& messageLite) noexcept -> std::vector<u_int8_t>{
        std::vector<u_int8_t> encoded(messageLite.ByteSizeLong());
        messageLite.SerializeToArray(encoded.data(), messageLite.ByteSizeLong());
        return encoded;
    }
}
#endif