#include <GATT/BluetoothCharacteristic.h>
#include <GATT/BluetoothDescriptor.h>

namespace btGatt{
    BluetoothCharacteristic::BluetoothCharacteristic(bt_gatt_h handle, std::weak_ptr<BluetoothService> service):
    _handle(handle),
    _service(service){}

    auto BluetoothCharacteristic::toProtoCharacteristic() const noexcept -> proto::gen::BluetoothCharacteristic{
        proto::gen::BluetoothCharacteristic proto;
        for(const auto& descriptor:_descriptors){
            *proto.add_descriptors()=descriptor.toProtoDescriptor();
        }
        return proto;
    }
}