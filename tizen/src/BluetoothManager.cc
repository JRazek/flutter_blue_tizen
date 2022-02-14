#include <BluetoothManager.h>

#include <system_info.h>
#include <Logger.h>
#include <LogLevel.h>

#include <bluetooth.h>
#include <tizen.h>

#include <BluetoothDeviceController.h>
#include <GATT/BluetoothDescriptor.h>


namespace btu{
    using LogLevel = btlog::LogLevel;
    using Logger = btlog::Logger;
    auto decodeAdvertisementData(char* packetsData, proto::gen::AdvertisementData& adv, int dataLen) -> void;


    BluetoothManager::BluetoothManager(NotificationsHandler& notificationsHandler) noexcept:
    _notificationsHandler(notificationsHandler){
        if(!isBLEAvailable()){
            Logger::log(LogLevel::ERROR, "Bluetooth is not available on this device!");
            return;
        }
        if(bt_initialize()){
            Logger::log(LogLevel::ERROR, "[bt_initialize] failed");
            return;
        }
        if(bt_gatt_set_connection_state_changed_cb(&BluetoothDeviceController::connectionStateCallback, this)) {
            Logger::log(LogLevel::ERROR, "[bt_device_set_bond_destroyed_cb] failed");
            return;
        }
        Logger::log(LogLevel::DEBUG, "All callbacks successfully initialized.");
    }

    auto BluetoothManager::setNotification(const proto::gen::SetNotificationRequest& request) -> void {
        std::scoped_lock lock(_bluetoothDevices.mut);
        
        auto characteristic=locateCharacteristic(request.remote_id(), request.service_uuid(), request.secondary_service_uuid(), request.characteristic_uuid());
        if(characteristic){
            if(request.enable())
                characteristic->setNotifyCallback([](auto& characteristic){
                    Logger::log(LogLevel::DEBUG, "notification from characteristic of uuid="+characteristic.UUID()+" to value="+characteristic.value());
                });
            else
                characteristic->unsetNotifyCallback();
        }else{
            throw BTException("could not find characteristic");
        }
    }

    auto BluetoothManager::getMtu(const std::string& deviceID) -> u_int32_t {
        std::scoped_lock lock(_bluetoothDevices.mut);
        auto device=_bluetoothDevices.var.find(deviceID)->second;
        if(!device)
            throw btu::BTException("could not find device of id="+deviceID);
            
        return device->getMtu();
    }

    auto BluetoothManager::requestMtu(const proto::gen::MtuSizeRequest& request) -> void {
        auto device=_bluetoothDevices.var.find(request.remote_id())->second;
        if(!device)
            throw btu::BTException("could not find device of id="+request.remote_id());

        device->requestMtu(request.mtu(), [](auto status, auto& bluetoothDevice){
            proto::gen::MtuSizeResponse res;
            res.set_remote_id(bluetoothDevice.cAddress());
            res.set_mtu(bluetoothDevice.getMtu());

            bluetoothDevice.cNotificationsHandler().notifyUIThread("MtuSize", res);
        });
    }

    auto BluetoothManager::locateCharacteristic(const std::string& remoteID, const std::string& primaryUUID,
    const std::string& secondaryUUID, const std::string& characteristicUUID) -> btGatt::BluetoothCharacteristic*{
        auto it=_bluetoothDevices.var.find(remoteID);
        if(it!=_bluetoothDevices.var.end()){
            auto device=it->second;
            auto primary=device->getService(primaryUUID);
            btGatt::BluetoothService* service=primary;
            if(primary && !secondaryUUID.empty()){
                service=primary->getSecondary(secondaryUUID);
            }
            if(service){
                return service->getCharacteristic(characteristicUUID);
            }
        }
        return nullptr;
    }
    auto BluetoothManager::locateDescriptor(const std::string& remoteID, const std::string& primaryUUID, const std::string& secondaryUUID, 
    const std::string& characteristicUUID, const std::string& descriptorUUID) -> btGatt::BluetoothDescriptor* {
        auto characteristic=locateCharacteristic(remoteID, primaryUUID, secondaryUUID, characteristicUUID);
        if(characteristic){
            return characteristic->getDescriptor(descriptorUUID);
        }
        return nullptr;
    }

    auto BluetoothManager::isBLEAvailable() noexcept -> bool{
        bool state;
        int ret = system_info_get_platform_bool("http://tizen.org/feature/network.bluetooth.le", &state);
        if(ret != SYSTEM_INFO_ERROR_NONE){
            Logger::log(LogLevel::ERROR, "failed fetching bluetooth data!");
        }
        return state;
    }

    auto BluetoothManager::bluetoothState() const noexcept -> proto::gen::BluetoothState{
        /* Checks whether the Bluetooth adapter is enabled */
        bt_adapter_state_e adapter_state;
        int ret = bt_adapter_get_state(&adapter_state);
        proto::gen::BluetoothState bts;
        if(ret == BT_ERROR_NONE){
            if(adapter_state == BT_ADAPTER_ENABLED)
                bts.set_state(proto::gen::BluetoothState_State_ON);
            else
                bts.set_state(proto::gen::BluetoothState_State_OFF);
        }
        else if(ret == BT_ERROR_NOT_INITIALIZED)
            bts.set_state(proto::gen::BluetoothState_State_UNAVAILABLE);
        else
            bts.set_state(proto::gen::BluetoothState_State_UNKNOWN);
        
        return bts;
    }

    auto BluetoothManager::adapterStateChangedCallback(int result, bt_adapter_state_e adapter_state, void* user_data) noexcept -> void {
        BluetoothManager& bluetoothManager = *static_cast<BluetoothManager*> (user_data);
        std::scoped_lock lock(bluetoothManager.adapterState.mut);
        bluetoothManager.adapterState.var = adapter_state;
    }
    
    auto BluetoothManager::startBluetoothDeviceScanLE(const proto::gen::ScanSettings& scanSettings) noexcept -> void {
        std::scoped_lock l(_bluetoothDevices.mut, _scanAllowDuplicates.mut);
        _bluetoothDevices.var.clear();        
        _scanAllowDuplicates.var=scanSettings.allow_duplicates();        
        Logger::log(LogLevel::DEBUG, "allowDuplicates="+std::to_string(_scanAllowDuplicates.var));

        int res = bt_adapter_le_set_scan_mode(BT_ADAPTER_LE_SCAN_MODE_BALANCED);
        //remove from bluetooth Devices all devices with status==Scanned

        int uuidCount=scanSettings.service_uuids_size();
        std::vector<bt_scan_filter_h> filters(uuidCount);

        for(int i=0;i<uuidCount;i++){
            const std::string& uuid=scanSettings.service_uuids()[i];
            bt_adapter_le_scan_filter_create(&filters[i]);
            bt_adapter_le_scan_filter_set_device_address(filters[i], uuid.c_str());
        }

        if(!res){
            btlog::Logger::log(btlog::LogLevel::DEBUG, "starting scan...");
            res = bt_adapter_le_start_scan(&BluetoothManager::scanCallback, this);
        }
        if(res){
            std::string err=get_error_message(res);
            Logger::log(LogLevel::ERROR, "scanning start failed with " + err);
        }
        for(auto& f : filters){
            bt_adapter_le_scan_filter_destroy(&f);
        }
    }

    auto BluetoothManager::scanCallback(int result, bt_adapter_le_device_scan_result_info_s *discovery_info, void *user_data) noexcept -> void {
        BluetoothManager& bluetoothManager = *static_cast<BluetoothManager*> (user_data);
        using State=BluetoothDeviceController::State;
        if(result)
            Logger::log(LogLevel::ERROR, "NULLPTR IN CALL!");
        else{
            std::string macAddress=discovery_info->remote_address;
            std::scoped_lock lock(bluetoothManager._bluetoothDevices.mut, bluetoothManager._scanAllowDuplicates.mut);
            std::shared_ptr<BluetoothDeviceController> device;
            if(bluetoothManager._bluetoothDevices.var.find(macAddress)==bluetoothManager._bluetoothDevices.var.end())
                device=bluetoothManager._bluetoothDevices.var.insert({macAddress, std::make_shared<BluetoothDeviceController>(macAddress, bluetoothManager._notificationsHandler)}).first->second;
            else
                device=bluetoothManager._bluetoothDevices.var.find(macAddress)->second;
                
            if(bluetoothManager._scanAllowDuplicates.var || device->cProtoBluetoothDevices().empty()){
                device->protoBluetoothDevices().emplace_back();
                            
                auto& protoDev=device->protoBluetoothDevices().back();
                char* name;
                result=bt_adapter_le_get_scan_result_device_name(discovery_info, BT_ADAPTER_LE_PACKET_SCAN_RESPONSE, &name);
                if(!result){
                    protoDev.set_name(name);
                    free(name);
                }
                protoDev.set_remote_id(macAddress);

                proto::gen::ScanResult scanResult;
                scanResult.set_rssi(discovery_info->rssi);
                proto::gen::AdvertisementData* advertisement_data=new proto::gen::AdvertisementData();
                decodeAdvertisementData(discovery_info->adv_data, *advertisement_data, discovery_info->adv_data_len);

                scanResult.set_allocated_advertisement_data(advertisement_data);
                scanResult.set_allocated_device(new proto::gen::BluetoothDevice(protoDev));

                bluetoothManager._notificationsHandler.notifyUIThread("ScanResult", scanResult);
            }
        }
    }
    
    auto BluetoothManager::stopBluetoothDeviceScanLE() noexcept -> void{
        static std::mutex m;
        std::scoped_lock lock(m);
        auto btState=bluetoothState().state();
        if(btState==proto::gen::BluetoothState_State_ON){
            bool isDiscovering;
            int res = bt_adapter_le_is_discovering(&isDiscovering);
            if(!res && isDiscovering){
                res = bt_adapter_le_stop_scan();
                if(!res) btlog::Logger::log(btlog::LogLevel::DEBUG, "scan stopped.");
                Logger::showResultError("bt_adapter_le_stop_scan", res);
            }
            Logger::showResultError("bt_adapter_le_is_discovering", res);

        }else{
            Logger::log(LogLevel::ERROR, "bluetooth adapter state="+std::to_string(btState));
        }
    }

    auto BluetoothManager::connect(const proto::gen::ConnectRequest& connRequest) noexcept -> void {
        std::unique_lock lock(_bluetoothDevices.mut);
        using State=BluetoothDeviceController::State;
        auto device=_bluetoothDevices.var.find(connRequest.remote_id())->second;
        device->connect(connRequest);
    }

    auto BluetoothManager::disconnect(const std::string& deviceID) noexcept -> void {
        std::unique_lock lock(_bluetoothDevices.mut);
        using State=BluetoothDeviceController::State;
        auto device=_bluetoothDevices.var.find(deviceID)->second;
        device->disconnect();
    }
    
    auto BluetoothManager::getConnectedProtoBluetoothDevices() noexcept -> std::vector<proto::gen::BluetoothDevice> {
        std::vector<proto::gen::BluetoothDevice> protoBD;
        std::scoped_lock lock(_bluetoothDevices.mut);
        using State=BluetoothDeviceController::State;
        for(const auto& e:_bluetoothDevices.var){
            if(e.second->state()==State::CONNECTED){
                auto& vec=e.second->cProtoBluetoothDevices();
                protoBD.insert(protoBD.end(), vec.cbegin(), vec.cend());
            }
        }
        return protoBD;
    }

    auto BluetoothManager::bluetoothDevices() noexcept -> decltype(_bluetoothDevices)& { return _bluetoothDevices; }

    auto BluetoothManager::readCharacteristic(const proto::gen::ReadCharacteristicRequest& request) -> void {
        using namespace btGatt;
        std::scoped_lock lock(_bluetoothDevices.mut);
        auto characteristic=locateCharacteristic(request.remote_id(), request.service_uuid(), request.secondary_service_uuid(), request.characteristic_uuid());
        if(characteristic){
            characteristic->read([](auto& characteristic)-> void {
                Logger::log(LogLevel::DEBUG, "cb called char ");
                proto::gen::ReadCharacteristicResponse res;
                res.set_remote_id(characteristic.cService().cDevice().cAddress());
                res.set_allocated_characteristic(new proto::gen::BluetoothCharacteristic(characteristic.toProtoCharacteristic()));
                
                Logger::log(LogLevel::DEBUG, "read value of characteristic="+characteristic.value());

                characteristic.cService().cDevice().cNotificationsHandler()
                                    .notifyUIThread("ReadCharacteristicResponse", res);
            });
            Logger::log(LogLevel::DEBUG, "read call!");
        }else{
            Logger::log(LogLevel::ERROR, "could not locate characteristic "+request.characteristic_uuid());
            throw BTException("could not locate characteristic");
        }
    }

    auto BluetoothManager::readDescriptor(const proto::gen::ReadDescriptorRequest& request) -> void {
        using namespace btGatt;
        std::scoped_lock lock(_bluetoothDevices.mut);
        auto descriptor=locateDescriptor(request.remote_id(), request.service_uuid(), request.secondary_service_uuid(), request.characteristic_uuid(), request.descriptor_uuid());
        if(descriptor){
            descriptor->read([](auto& descriptor)-> void {
                proto::gen::ReadDescriptorRequest* request=new proto::gen::ReadDescriptorRequest();
                request->set_remote_id(descriptor.cCharacteristic().cService().cDevice().cAddress());
                request->set_characteristic_uuid(descriptor.cCharacteristic().UUID());
                request->set_descriptor_uuid(descriptor.UUID());

                if(descriptor.cCharacteristic().cService().getType()==btGatt::ServiceType::PRIMARY){
                    request->set_service_uuid(descriptor.cCharacteristic().cService().UUID());
                }else{
                    auto& secondary=dynamic_cast<const btGatt::SecondaryService&>(descriptor.cCharacteristic().cService());
                    request->set_service_uuid(secondary.cPrimary().UUID());
                    request->set_secondary_service_uuid(secondary.UUID());
                }

                proto::gen::ReadDescriptorResponse res;
                res.set_allocated_request(request);
                res.set_allocated_value(new std::string(descriptor.value()));
                descriptor.cCharacteristic().cService().cDevice().cNotificationsHandler()
                                .notifyUIThread("ReadDescriptorResponse", res);
            });
        }else{
            Logger::log(LogLevel::ERROR, "could not locate descriptor "+request.characteristic_uuid());
            throw BTException("could not locate descriptor");
        }
    }

    auto BluetoothManager::writeCharacteristic(const proto::gen::WriteCharacteristicRequest& request) -> void {
        using namespace btGatt;
        std::scoped_lock lock(_bluetoothDevices.mut);
        auto characteristic=locateCharacteristic(request.remote_id(), request.service_uuid(), request.secondary_service_uuid(), request.characteristic_uuid());
        if(characteristic){
            Logger::log(LogLevel::DEBUG, "writing to "+characteristic->cService().cDevice().cAddress()+"...");
            characteristic->write(request.value(), request.write_type(), [](bool success, auto& characteristic){
                Logger::log(LogLevel::DEBUG, "characteristic write callback!");
                proto::gen::WriteCharacteristicResponse res;
                proto::gen::WriteCharacteristicRequest* request=new proto::gen::WriteCharacteristicRequest();
                request->set_remote_id(characteristic.cService().cDevice().cAddress());
                request->set_characteristic_uuid(characteristic.UUID());
                
                if(characteristic.cService().getType()==btGatt::ServiceType::PRIMARY){
                    request->set_service_uuid(characteristic.cService().UUID());
                }else{
                    auto& secondary=dynamic_cast<const btGatt::SecondaryService&>(characteristic.cService());
                    request->set_service_uuid(secondary.cPrimary().UUID());
                    request->set_secondary_service_uuid(secondary.UUID());
                }
                res.set_success(success);
                res.set_allocated_request(request);
                characteristic.cService().cDevice().cNotificationsHandler()
                                .notifyUIThread("WriteCharacteristicResponse", res);

            });
            Logger::log(LogLevel::DEBUG, "called async write...");
        }else{
            Logger::log(LogLevel::ERROR, "could not locate characteristic "+request.characteristic_uuid());
            throw BTException("could not locate characteristic");
        }
    }

    auto BluetoothManager::writeDescriptor(const proto::gen::WriteDescriptorRequest& request) -> void {
        std::scoped_lock lock(_bluetoothDevices.mut);
        auto descriptor=locateDescriptor(request.remote_id(), request.service_uuid(), request.secondary_service_uuid(), request.characteristic_uuid(), request.descriptor_uuid());
        if(descriptor){
            descriptor->write(request.value(), [](auto success, auto& descriptor) -> void {
                Logger::log(LogLevel::DEBUG, "descriptor write callback!");
                proto::gen::WriteDescriptorRequest* request=new proto::gen::WriteDescriptorRequest();

                if(descriptor.cCharacteristic().cService().getType()==btGatt::ServiceType::PRIMARY){
                    request->set_service_uuid(descriptor.cCharacteristic().cService().UUID());
                }else{
                    auto& secondary=dynamic_cast<const btGatt::SecondaryService&>(descriptor.cCharacteristic().cService());
                    request->set_service_uuid(secondary.cPrimary().UUID());
                    request->set_secondary_service_uuid(secondary.UUID());
                }
                request->set_descriptor_uuid(descriptor.UUID());
                request->set_remote_id(descriptor.cCharacteristic().cService().cDevice().cAddress());
                request->set_characteristic_uuid(descriptor.cCharacteristic().UUID());
                
                proto::gen::WriteDescriptorResponse res;
                res.set_success(success);
                res.set_allocated_request(request);

                descriptor.cCharacteristic().cService().cDevice().cNotificationsHandler()
                                .notifyUIThread("WriteDescriptorResponse", res);
            });
        }else{
            Logger::log(LogLevel::ERROR, "could not locate descriptor "+request.characteristic_uuid());
            throw BTException("could not locate descriptor");
        }
        
    }

    auto decodeAdvertisementData(char* packetsData, proto::gen::AdvertisementData& adv, int dataLen) -> void {
        using byte=char;
        int start=0;
        bool longNameSet=false;
        while(start<dataLen){
            byte ad_len=packetsData[start]&0xFFu;
            byte type=packetsData[start+1]&0xFFu;

            byte* packet=packetsData+start+2;
            switch(type){
                case 0x09:
                case 0x08:{
                    if(!longNameSet) {
                        adv.set_local_name(packet, ad_len-1);
                    }
                    
                    if(type==0x09) longNameSet=true;
                    break;
                }
                case 0x01:{
                    adv.set_connectable(*packet & 0x3);
                    break;
                }
                case 0xFF:{
                    break;
                }
                default: break;
            }
            start+=ad_len+1;
        }
    }


} // namespace btu

