#include <GATT/BluetoothService.h>
#include <GATT/BluetoothCharacteristic.h>
#include <BluetoothDeviceController.h>
#include <Logger.h>

#include <bluetooth.h>

namespace btGatt{
    using namespace btu;
    using namespace btlog;

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
    _primaryService(primaryService){

    }
    
    auto PrimaryService::cDevice() const noexcept -> const btu::BluetoothDeviceController&{
        return _device;
    }
    auto PrimaryService::toProtoService() const noexcept -> proto::gen::BluetoothService {
        proto::gen::BluetoothService proto;
        proto.set_remote_id(_device.cAddress());
        proto.set_uuid(getGattUUID(_handle));
        proto.set_is_primary(true);
        for(const auto& characteristic : _characteristics){
            *proto.add_characteristics()=characteristic.toProtoCharacteristic();
        }
        for(const auto& secondary : _secondaryServices){
            *proto.add_included_services()=secondary.toProtoService();
        }
        return proto;
    }
    auto PrimaryService::getType() const noexcept -> ServiceType {
        return ServiceType::PRIMARY;
    }

    auto SecondaryService::cDevice() const noexcept -> const btu::BluetoothDeviceController&{
        return _primaryService.cDevice();
    }
    auto SecondaryService::cPrimary() const noexcept -> const decltype(_primaryService)&{
        return _primaryService;
    }
    auto SecondaryService::toProtoService() const noexcept -> proto::gen::BluetoothService {
        proto::gen::BluetoothService proto;
        proto.set_remote_id(_primaryService.cDevice().cAddress());
        proto.set_uuid(getGattUUID(_handle));
        proto.set_is_primary(false);
        for(const auto& characteristic : _characteristics){
            *proto.add_characteristics()=characteristic.toProtoCharacteristic();
        }
        return proto;
    }

    auto SecondaryService::getType() const noexcept -> ServiceType {
        return ServiceType::SECONDARY;
    }
    auto SecondaryService::primaryUUID() noexcept -> std::string {
        return _primaryService.UUID();
    }

    auto BluetoothService::getProtoProperties() const noexcept -> proto::gen::CharacteristicProperties {
        proto::gen::CharacteristicProperties p;
        int properties=0;
        int res=bt_gatt_characteristic_get_properties(_handle, &properties);
        Logger::showResultError("bt_gatt_characteristic_get_properties", res);
        if(!res){
            p.set_broadcast((properties & 1) != 0);
            p.set_read((properties & 2) != 0);
            p.set_write_without_response((properties & 4) != 0);
            p.set_write((properties & 8) != 0);
            p.set_notify((properties & 16) != 0);
            p.set_indicate((properties & 32) != 0);
            p.set_authenticated_signed_writes((properties & 64) != 0);
            p.set_extended_properties((properties & 128) != 0);
            p.set_notify_encryption_required((properties & 256) != 0);
            p.set_indicate_encryption_required((properties & 512) != 0);
        }
        return p;
    }
    auto BluetoothService::UUID() noexcept -> std::string {
        return getGattUUID(_handle);
    }

}