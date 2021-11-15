#ifndef BLUETOOTH_STATE_H
#define BLUETOOTH_STATE_H

namespace btu{
    enum class BluetoothState{
        OFF,
        ON,
        TURNING_OFF,
        TURNING_ON,
        UNKNOWN,
        UNAUTHORIZED
    };
}
#endif //BLUETOOTH_STATE_H