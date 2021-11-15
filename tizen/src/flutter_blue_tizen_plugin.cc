#include "flutter_blue_tizen_plugin.h"

// For getPlatformVersion; remove unless needed for your plugin implementation.
#include <system_info.h>

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_method_codec.h>

#include <map>
#include <memory>
#include <sstream>
#include <string>
#include "BluetoothUtils.h"
#include "log.h"

namespace {

class FlutterBlueTizenPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrar *registrar) {
    auto channel =
        std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
            registrar->messenger(), "flutter_blue_tizen",
            &flutter::StandardMethodCodec::GetInstance());

    auto plugin = std::make_unique<FlutterBlueTizenPlugin>();

    channel->SetMethodCallHandler(
        [plugin_pointer = plugin.get()](const auto &call, auto result) {
          plugin_pointer->HandleMethodCall(call, std::move(result));
        });

    registrar->AddPlugin(std::move(plugin));
  }

  btu::BluetoothUtils bluetoothUtils;

  FlutterBlueTizenPlugin():
  bluetoothUtils()
  {}

  virtual ~FlutterBlueTizenPlugin() {}

 private:
  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    if(method_call.method_name() == "isAvailable"){
        result->Success(flutter::EncodableValue(bluetoothUtils.getBluetoothAvailability()));
    }else {
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
