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

#include <flutterblue.pb.h>

namespace {

  static std::vector<u_int8_t> encodeToVector(const google::protobuf::MessageLite& messageLite);
  
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

    btu::BluetoothManager bluetoothManager;

    FlutterBlueTizenPlugin():
    bluetoothManager(methodChannel)
    {}

    virtual ~FlutterBlueTizenPlugin() {}

  private:
    void HandleMethodCall(const flutter::MethodCall<flutter::EncodableValue> &method_call, std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
      const flutter::EncodableValue& args = *method_call.arguments();
      if(method_call.method_name() == "isAvailable"){
          result->Success(flutter::EncodableValue(bluetoothManager.getBluetoothAvailabilityLE()));
      }
      else if(method_call.method_name() == "setLogLevel" && std::holds_alternative<int>(args)){
          log_priority logLevel = static_cast<log_priority>(std::get<int>(args));
          btlog::Logger::setLogLevel(logLevel);
          result->Success(flutter::EncodableValue(NULL));
      }
      else if(method_call.method_name() == "state"){
          result->Success(flutter::EncodableValue(encodeToVector(bluetoothManager.getBluetoothState())));
      }
      else if(method_call.method_name() == "isOn"){
          result->Success(flutter::EncodableValue((bluetoothManager.getBluetoothState().state() == BluetoothState_State::BluetoothState_State_ON)));
      }
      else if(method_call.method_name() == "startScan"){
          ScanSettings scanSettings;
          bluetoothManager.startBluetoothDeviceScanLE(scanSettings);
          btlog::Logger::log(btlog::LogLevel::DEBUG, "starting scan...");
          result->Success(flutter::EncodableValue(NULL));
      }
      else if(method_call.method_name() == "stopScan"){
          bluetoothManager.stopBluetoothDeviceScanLE();
          result->Success(flutter::EncodableValue(NULL));
      }
      else if(method_call.method_name() == "getConnectedDevices"){
          ConnectedDevicesResponse response;
          auto& p = bluetoothManager.getConnectedDevicesLE();
          {
            std::scoped_lock lock(p.mut);
            for(const auto& dev : p.var){
              BluetoothDevice* bluetoothDevice = response.add_devices();
              *bluetoothDevice = dev;
            }
          }
          //[TODO] TEST THIS FUNCTION
          result->Success(flutter::EncodableValue(encodeToVector(response)));
      }
      else if(method_call.method_name() == "connect"){
        
      }
      else if(method_call.method_name() == "disconnect"){
        
      }
      else if(method_call.method_name() == "deviceState"){
        
      }
      else if(method_call.method_name() == "discoverServices"){
        
      }
      else {
        result->NotImplemented();
      }
    }
  };

  static std::vector<u_int8_t> encodeToVector(const google::protobuf::MessageLite& messageLite){
    std::vector<u_int8_t> encoded(messageLite.ByteSizeLong());
    messageLite.SerializeToArray(encoded.data(), messageLite.ByteSizeLong());
    return encoded;
  } 
}  // namespace

void FlutterBlueTizenPluginRegisterWithRegistrar(FlutterDesktopPluginRegistrarRef registrar) { 
      FlutterBlueTizenPlugin::RegisterWithRegistrar(flutter::PluginRegistrarManager::GetInstance()->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
