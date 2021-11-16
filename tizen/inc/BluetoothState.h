#ifndef BLUETOOTH_STATE_H
#define BLUETOOTH_STATE_H

namespace btu{
    enum class BluetoothState{
        OFF,
        ON,
        UNKNOWN,
        UNAUTHORIZED,
        UNINITIALIZED
    };
}
#endif //BLUETOOTH_STATE_H