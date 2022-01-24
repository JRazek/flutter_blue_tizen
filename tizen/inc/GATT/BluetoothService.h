#ifndef BLEUTOOTH_SERVICE_H
#define BLEUTOOTH_SERVICE_H

#include <bluetooth.h>

#include <unordered_map>
#include <memory>
#include <vector>

namespace btGatt{
    class BluetoothDeviceController;
    class BluetoothCharacteristic;
    class SecondaryService;
    class BluetoothService{
    protected:
        bt_gatt_h _handle;
        std::weak_ptr<BluetoothDeviceController> _device;
        std::vector<BluetoothCharacteristic> _characteristics;

        BluetoothService(bt_gatt_h handle, std::weak_ptr<BluetoothDeviceController> device);
    public:
        virtual auto toProtoService() -> void=0;
    };



    ///////PRIMARY///////
    class PrimaryService : public BluetoothService{
        std::shared_ptr<SecondaryService> _primaryService;

    public:
        PrimaryService(bt_gatt_h handle, std::weak_ptr<BluetoothDeviceController> device);
    };

    ///////SECONDARY///////
    class SecondaryService : public BluetoothService{
        std::shared_ptr<PrimaryService> _primaryService;
    public:
        SecondaryService(bt_gatt_h service_handle, std::weak_ptr<BluetoothDeviceController> device, std::shared_ptr<PrimaryService> primaryService);
    };
}
#endif //BLEUTOOTH_SERVICE_H