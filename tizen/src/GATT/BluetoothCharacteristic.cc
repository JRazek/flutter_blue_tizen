#include <GATT/BluetoothCharacteristic.h>
#include <GATT/BluetoothDescriptor.h>
#include <GATT/BluetoothService.h>
#include <BluetoothDeviceController.h>
#include <Utils.h>

namespace btGatt{
    using namespace btu;
    BluetoothCharacteristic::BluetoothCharacteristic(bt_gatt_h handle, BluetoothService& service):
    _handle(handle),
    _service(service){
        int res=bt_gatt_characteristic_foreach_descriptors(handle, [](int total, int index, bt_gatt_h descriptor_handle, void* scope_ptr) -> bool {
            auto& characteristic=*static_cast<BluetoothCharacteristic*>(scope_ptr);
            characteristic._descriptors.emplace_back(descriptor_handle, characteristic);
            return true;
        }, this);
    }

    auto BluetoothCharacteristic::toProtoCharacteristic() const noexcept -> proto::gen::BluetoothCharacteristic{
        proto::gen::BluetoothCharacteristic proto;
        proto.set_remote_id(_service.cDevice().cAddress());
        proto.set_uuid(getGattUUID(_handle));
        proto.set_allocated_properties(new proto::gen::CharacteristicProperties(getProtoCharacteristicProperties(_handle)));
        //value?
        if(_service.getType()==ServiceType::PRIMARY)
            proto.set_serviceuuid(_service.UUID());
        else{
            SecondaryService& sec=dynamic_cast<SecondaryService&>(_service);
            proto.set_serviceuuid(sec.UUID());
            proto.set_secondaryserviceuuid(sec.primaryUUID());
        }
        for(const auto& descriptor:_descriptors){
            *proto.add_descriptors()=descriptor.toProtoDescriptor();
        }
        return proto;
    }
    auto BluetoothCharacteristic::cService() const noexcept -> const decltype(_service)&{
        return _service;
    }
    auto BluetoothCharacteristic::UUID() const noexcept -> std::string {
        return getGattUUID(_handle);
    }

}