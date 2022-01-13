#ifndef UTILS_H
#define UTILS_H

#include <flutter/method_channel.h>
#include <flutter/encodable_value.h>
#include <flutterblue.pb.h>

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
}
#endif