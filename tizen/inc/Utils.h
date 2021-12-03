
#ifndef UTILS_H
#define UTILS_H

#include <mutex>
namespace btu{
    template<typename T>
    struct SafeType{
        T var;
        std::mutex mut;
    };
}
#endif