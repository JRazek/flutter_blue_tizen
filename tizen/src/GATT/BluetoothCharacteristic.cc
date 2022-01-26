#include <GATT/BluetoothCharacteristic.h>
#include <GATT/BluetoothDescriptor.h>

namespace btGatt{
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
        for(const auto& descriptor:_descriptors){
            *proto.add_descriptors()=descriptor.toProtoDescriptor();
        }
        return proto;
    }
}