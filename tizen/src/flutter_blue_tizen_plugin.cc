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

  FlutterBlueTizenPlugin() {}

  virtual ~FlutterBlueTizenPlugin() {}

 private:
  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    // Replace "getPlatformVersion" check with your plugin's method.
    if (method_call.method_name().compare("getPlatformVersion") == 0) {
      char *value = NULL;
      int ret = system_info_get_platform_string(
          "http://tizen.org/feature/platform.version", &value);
      if (ret == SYSTEM_INFO_ERROR_NONE) {
        result->Success(flutter::EncodableValue(std::string(value)));
      } else {
        result->Error(std::to_string(ret), "Failed to get platform version.");
      }
      if (value) {
        free(value);
      }
    } else {
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
