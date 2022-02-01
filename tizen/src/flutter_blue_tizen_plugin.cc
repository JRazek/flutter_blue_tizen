#include "flutter_blue_tizen_plugin.h"

#include <system_info.h>
#include <app_control.h>
#include <bluetooth.h>

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_method_codec.h>

#include <map>
#include <memory>
#include <sstream>
#include <string>

#include <BluetoothManager.h>
#include <Logger.h>
#include <BluetoothDeviceController.h>
#include <NotificationsHandler.h>

#include <flutterblue.pb.h>

namespace {
  class FlutterBlueTizenPlugin : public flutter::Plugin {
  public:
    const static inline std::string channel_name = "plugins.pauldemarco.com/flutter_blue/";
    
    static inline std::shared_ptr<flutter::MethodChannel<flutter::EncodableValue>> methodChannel;

    static void RegisterWithRegistrar(flutter::PluginRegistrar *registrar) {
      methodChannel = std::make_shared<flutter::MethodChannel<flutter::EncodableValue>>(registrar->messenger(), (channel_name + "methods"), &flutter::StandardMethodCodec::GetInstance());

      auto plugin = std::make_unique<FlutterBlueTizenPlugin>();

      methodChannel->SetMethodCallHandler([plugin_pointer = plugin.get()] (const auto &call, auto result) {
        plugin_pointer->HandleMethodCall(call, std::move(result));
      });

      registrar->AddPlugin(std::move(plugin));
    }

    btu::NotificationsHandler notificationsHandler;
    btu::BluetoothManager bluetoothManager;
    FlutterBlueTizenPlugin():
    notificationsHandler(methodChannel),
    bluetoothManager(notificationsHandler)
    {}

    virtual ~FlutterBlueTizenPlugin() {
      google::protobuf::ShutdownProtobufLibrary();
      // if(bt_deinitialize()){
      //     btlog::Logger::log(btlog::LogLevel::ERROR, "[bt_deinitialize] failed");
      // }
    }

  private:
    void HandleMethodCall(const flutter::MethodCall<flutter::EncodableValue> &method_call, std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
      const flutter::EncodableValue& args = *method_call.arguments();
      if(method_call.method_name() == "isAvailable"){
          result->Success(flutter::EncodableValue(bluetoothManager.isBLEAvailable()));
      }
      else if(method_call.method_name() == "setLogLevel" && std::holds_alternative<int>(args)){
          btlog::LogLevel logLevel = static_cast<btlog::LogLevel>(std::get<int>(args));
          btlog::Logger::setLogLevel(logLevel);
          result->Success(flutter::EncodableValue(NULL));
      }
      else if(method_call.method_name() == "state"){
          result->Success(flutter::EncodableValue(btu::messageToVector(bluetoothManager.bluetoothState())));
      }
      else if(method_call.method_name() == "isOn"){
          result->Success(flutter::EncodableValue((bluetoothManager.bluetoothState().state() == proto::gen::BluetoothState_State::BluetoothState_State_ON)));
      }
      else if(method_call.method_name() == "startScan"){
          proto::gen::ScanSettings scanSettings;
          std::vector<uint8_t> encoded = std::get<std::vector<uint8_t>>(args);
          scanSettings.ParseFromArray(encoded.data(), encoded.size());
          bluetoothManager.startBluetoothDeviceScanLE(scanSettings);
          result->Success(flutter::EncodableValue(NULL));
      }
      else if(method_call.method_name() == "stopScan"){
          bluetoothManager.stopBluetoothDeviceScanLE();
          result->Success(flutter::EncodableValue(NULL));
      }
      else if(method_call.method_name() == "getConnectedDevices"){
          proto::gen::ConnectedDevicesResponse response;
          auto p = bluetoothManager.getConnectedProtoBluetoothDevices();

          for(auto& dev : p){
            *response.add_devices()=std::move(dev);
          }
          
          //[TODO] TEST THIS FUNCTION
          result->Success(flutter::EncodableValue(btu::messageToVector(response)));
      }
      else if(method_call.method_name() == "connect"){
        std::vector<uint8_t> encoded = std::get<std::vector<uint8_t>>(args);
        proto::gen::ConnectRequest connectRequest;
        bool ok = connectRequest.ParseFromArray(encoded.data(), encoded.size());
        
        bluetoothManager.connect(connectRequest);
        if(!ok)
          result->Error("could not deserialize request!");
        else
          result->Success(flutter::EncodableValue(NULL));
      }
      else if(method_call.method_name() == "disconnect"){
        std::string deviceID = std::get<std::string>(args);

        bluetoothManager.disconnect(deviceID);
        result->Success(flutter::EncodableValue(NULL));
      }
      else if(method_call.method_name() == "deviceState"){
        std::string deviceID = std::get<std::string>(args);
        std::scoped_lock lock(bluetoothManager.bluetoothDevices().mut);
        auto it=bluetoothManager.bluetoothDevices().var.find(deviceID);
        if(it!=bluetoothManager.bluetoothDevices().var.end()){
          auto& device=(*it).second;

          proto::gen::DeviceStateResponse res;
          res.set_remote_id(device->cAddress());
          res.set_state(btu::localToProtoDeviceState(device->state()));

          result->Success(flutter::EncodableValue(btu::messageToVector(res)));
        } else result->Error("device not available");
      }
      else if(method_call.method_name() == "discoverServices"){
        std::string deviceID = std::get<std::string>(args);
        std::scoped_lock lock(bluetoothManager.bluetoothDevices().mut);
        auto it=bluetoothManager.bluetoothDevices().var.find(deviceID);
        if(it!=bluetoothManager.bluetoothDevices().var.end()){
          auto& device=it->second;
          result->Success(flutter::EncodableValue(NULL));

          auto services=device->discoverServices();
          notificationsHandler.notifyUIThread("DiscoverServicesResult", btu::getProtoServiceDiscoveryResult(*device.get(), services));
        }
        else 
            result->Error("device not available");
      }
      else if(method_call.method_name() == "readCharacteristic"){
        std::vector<uint8_t> encoded = std::get<std::vector<uint8_t>>(args);
        proto::gen::ReadCharacteristicRequest request;
        request.ParseFromArray(encoded.data(), encoded.size());
        bluetoothManager.readCharacteristic(request);
        result->Success(flutter::EncodableValue(NULL));
      }
      else if(method_call.method_name() == "readDescriptor"){
        std::vector<uint8_t> encoded = std::get<std::vector<uint8_t>>(args);
        proto::gen::ReadDescriptorRequest request;
        request.ParseFromArray(encoded.data(), encoded.size());
        bluetoothManager.readDescriptor(request);
        result->Success(flutter::EncodableValue(NULL));
      }
      else if(method_call.method_name() == "writeCharacteristic"){
        std::vector<uint8_t> encoded = std::get<std::vector<uint8_t>>(args);
        proto::gen::WriteCharacteristicRequest request;
        request.ParseFromArray(encoded.data(), encoded.size());
        bluetoothManager.writeCharacteristic(request);
        result->Success(flutter::EncodableValue(NULL));
      }
      else if(method_call.method_name() == "writeDescriptor"){
        std::vector<uint8_t> encoded = std::get<std::vector<uint8_t>>(args);
        proto::gen::WriteDescriptorRequest request;
        request.ParseFromArray(encoded.data(), encoded.size());
        bluetoothManager.writeDescriptor(request);
        result->Success(flutter::EncodableValue(NULL));
      }
      else {
        result->NotImplemented();
      }
    }
  };
}  // namespace

void FlutterBlueTizenPluginRegisterWithRegistrar(FlutterDesktopPluginRegistrarRef registrar) { 
      FlutterBlueTizenPlugin::RegisterWithRegistrar(flutter::PluginRegistrarManager::GetInstance()->GetRegistrar<flutter::PluginRegistrar>(registrar));
}

