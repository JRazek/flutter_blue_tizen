
#ifndef UTILS_H
#define UTILS_H

#include <mutex>
namespace btu{
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