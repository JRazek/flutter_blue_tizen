#ifndef BLEUTOOTH_SERVICE_H
#define BLEUTOOTH_SERVICE_H

#include <unordered_map>
#include <Utils.h>

namespace btGatt{
    class BluetoothDeviceController;
    class BluetoothService{
    protected:
        bt_gatt_h _handle;
        std::weak_ptr<BluetoothDeviceController> _device;

        BluetoothService(bt_gatt_h handle, std::weak_ptr<BluetoothDeviceController> device):
        _handle(handle),
        _device(device){}

    public:
        virtual auto toProtoService() -> void=0;
    };



    ///////PRIMARY///////
    class PrimaryService : public BluetoothService{
        std::shared_ptr<SecondaryService> _primaryService;

    public:
        PrimaryService(bt_gatt_h handle, std::weak_ptr<BluetoothDeviceController> device):
        BluetoothService(handle, device){}
    };

    ///////SECONDARY///////
    class SecondaryService : public BluetoothService{
        std::shared_ptr<PrimaryService> _primaryService;
    public:
        SecondaryService(bt_gatt_h service_handle, std::weak_ptr<BluetoothDeviceController> device, std::shared_ptr<PrimaryService> primaryService):
        BluetoothService(service_handle, device),
        _primaryService(primaryService){}
    };
}
#endif //BLEUTOOTH_SERVICE_H