#include <GATT/BluetoothService.h>
#include <GATT/BluetoothCharacteristic.h>
namespace btGatt{
    BluetoothService::BluetoothService(bt_gatt_h handle, std::weak_ptr<BluetoothDeviceController> device):
    _handle(handle),
    _device(device){
        int res=bt_gatt_service_foreach_characteristics(_handle, [](int total, int index, bt_gatt_h handle, void* _service) -> bool{
            auto& service=*static_cast<BluetoothService *>(_service);
            service._characteristics.emplace_back(handle, service);
            return true;
        }, this);
    }

    PrimaryService::PrimaryService(bt_gatt_h handle, std::weak_ptr<BluetoothDeviceController> device):
    BluetoothService(handle, device){}

    SecondaryService::SecondaryService(bt_gatt_h service_handle, std::weak_ptr<BluetoothDeviceController> device, std::shared_ptr<PrimaryService> primaryService):
    BluetoothService(service_handle, device),
    _primaryService(primaryService){}


    
    auto BluetoothService::toProtoService() const noexcept -> proto::gen::BluetoothService {
        proto::gen::BluetoothService proto;
        for(const auto& characteristic : _characteristics){
            *proto.add_characteristics()=characteristic.toProtoCharacteristic();
        }
        return proto;
    }
}