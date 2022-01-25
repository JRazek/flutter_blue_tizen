#ifndef BLEUTOOTH_SERVICE_H
#define BLEUTOOTH_SERVICE_H

#include <bluetooth.h>

#include <flutterblue.pb.h>

#include <unordered_map>
#include <memory>
#include <vector>

namespace btGatt{
    class BluetoothDeviceController;
    class BluetoothCharacteristic;
    class SecondaryService;
    class BluetoothService {
    protected:
        bt_gatt_h _handle;
        std::vector<BluetoothCharacteristic> _characteristics;

        BluetoothService(bt_gatt_h handle);
        BluetoothService(const BluetoothService&)=default;
        ~BluetoothService()=default;
    public:
        virtual auto toProtoService() const noexcept -> proto::gen::BluetoothService=0;
    };


    ///////PRIMARY///////
    class PrimaryService : public BluetoothService {
        BluetoothDeviceController& _device;
        std::vector<SecondaryService> _secondaryServices;

    public:
        PrimaryService(bt_gatt_h handle, BluetoothDeviceController& device);
        PrimaryService(const PrimaryService&)=default;

        auto toProtoService() const noexcept -> proto::gen::BluetoothService override;
    };

    ///////SECONDARY///////
    class SecondaryService : public BluetoothService {
        PrimaryService& _primaryService;
    public:
        SecondaryService(bt_gatt_h service_handle, PrimaryService& primaryService);
        SecondaryService(const SecondaryService&)=default;

        auto toProtoService() const noexcept -> proto::gen::BluetoothService override;
    };
}
#endif //BLEUTOOTH_SERVICE_H