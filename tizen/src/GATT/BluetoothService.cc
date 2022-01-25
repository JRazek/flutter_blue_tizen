#include <GATT/BluetoothService.h>
#include <GATT/BluetoothCharacteristic.h>
namespace btGatt{
    BluetoothService::BluetoothService(bt_gatt_h handle):
    _handle(handle){
        int res=bt_gatt_service_foreach_characteristics(_handle, [](int total, int index, bt_gatt_h handle, void* _service) -> bool{
            auto& service=*static_cast<BluetoothService *>(_service);
            service._characteristics.emplace_back(handle, service);
            return true;
        }, this);
    }

    PrimaryService::PrimaryService(bt_gatt_h handle, BluetoothDeviceController& device):
    BluetoothService(handle),
    _device(device){}

    SecondaryService::SecondaryService(bt_gatt_h service_handle, PrimaryService& primaryService):
    BluetoothService(service_handle),
    _primaryService(primaryService){}


    
    auto PrimaryService::toProtoService() const noexcept -> proto::gen::BluetoothService {
        proto::gen::BluetoothService proto;
        for(const auto& characteristic : _characteristics){
            *proto.add_characteristics()=characteristic.toProtoCharacteristic();
        }
        return proto;
    }
    
    auto SecondaryService::toProtoService() const noexcept -> proto::gen::BluetoothService {
        proto::gen::BluetoothService proto;
        for(const auto& characteristic : _characteristics){
            *proto.add_characteristics()=characteristic.toProtoCharacteristic();
        }
        return proto;
    }
}