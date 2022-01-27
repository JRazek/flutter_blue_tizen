#include <GATT/BluetoothDescriptor.h>
#include <GATT/BluetoothCharacteristic.h>
#include <BluetoothDeviceController.h>
#include <Utils.h>

namespace btGatt{
    using namespace btu;
    BluetoothDescriptor::BluetoothDescriptor(bt_gatt_h handle, BluetoothCharacteristic& characteristic):
    _handle(handle),
    _characteristic(characteristic){}
    
    auto BluetoothDescriptor::toProtoDescriptor() const noexcept -> proto::gen::BluetoothDescriptor{
        proto::gen::BluetoothDescriptor proto;
        proto.set_remote_id(_characteristic.cService().cDevice().cAddress());
        proto.set_serviceuuid(_characteristic.cService().UUID());
        proto.set_characteristicuuid(_characteristic.UUID());
        proto.set_uuid(UUID());
        return proto;
    }
    auto BluetoothDescriptor::UUID() const noexcept -> std::string {
        return getGattUUID(_handle);
    }

}