#include "flutter_blue_tizen_plugin.h"

// For getPlatformVersion; remove unless needed for your plugin implementation.
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

class FlutterBlueTizenPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrar *registrar) {
    auto channel =
        std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
            registrar->messenger(), "plugins.pauldemarco.com/flutter_blue/methods",
            &flutter::StandardMethodCodec::GetInstance());

    auto plugin = std::make_unique<FlutterBlueTizenPlugin>();

    channel->SetMethodCallHandler(
        [plugin_pointer = plugin.get()](const auto &call, auto result) {
          plugin_pointer->HandleMethodCall(call, std::move(result));
        });

    registrar->AddPlugin(std::move(plugin));
  }

  btu::BluetoothManager bluetoothManager;

  FlutterBlueTizenPlugin():
  bluetoothManager()
  {}

  virtual ~FlutterBlueTizenPlugin() {}

 private:
  void HandleMethodCall(const flutter::MethodCall<flutter::EncodableValue> &method_call, std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    const flutter::EncodableValue& args = *method_call.arguments();
    if(method_call.method_name() == "isAvailable"){
        result->Success(flutter::EncodableValue(bluetoothManager.getBluetoothAvailability()));
    }
    else if(method_call.method_name() == "setLogLevel" && std::holds_alternative<int>(args)){
        log_priority logLevel = static_cast<log_priority>(std::get<int>(args));
        btlog::Logger::setLogLevel(logLevel);
        result->Success(flutter::EncodableValue(true));
    }
    else if(method_call.method_name() == "state"){
        result->Success(flutter::EncodableValue(bluetoothManager.getBluetoothState().SerializeAsString()));
    }
    else if(method_call.method_name() == "isOn"){
        result->Success(flutter::EncodableValue((bluetoothManager.getBluetoothState().state() == BluetoothState_State::BluetoothState_State_ON)));
    }
    else if(method_call.method_name() == "startScan"){
        bluetoothManager.startBluetoothDeviceDiscovery();
    }
    else if(method_call.method_name() == "stopScan"){
        bluetoothManager.stopBluetoothDeviceDiscovery();
        result->Success(flutter::EncodableValue(NULL));
    }
    else if(method_call.method_name() == "getConnectedDevices"){
        ConnectedDevicesResponse response;
        for(const auto& dev : bluetoothManager.getDiscoveryDevices()){
          BluetoothDevice* bluetoothDevice = response.add_devices();
          *bluetoothDevice = dev;
        }
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

}  // namespace

void FlutterBlueTizenPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  FlutterBlueTizenPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}