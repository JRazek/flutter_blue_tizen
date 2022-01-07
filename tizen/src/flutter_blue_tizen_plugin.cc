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

#include <flutterblue.pb.h>

namespace {
  static auto encodeToVector(const google::protobuf::MessageLite& messageLite) noexcept -> std::vector<u_int8_t>;
  static auto fromLocalState(const btu::BluetoothDeviceController::State& s) -> DeviceStateResponse_BluetoothDeviceState;
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

    virtual ~FlutterBlueTizenPlugin() {
      google::protobuf::ShutdownProtobufLibrary();
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
          result->Success(flutter::EncodableValue(encodeToVector(bluetoothManager.bluetoothState())));
      }
      else if(method_call.method_name() == "isOn"){
          result->Success(flutter::EncodableValue((bluetoothManager.bluetoothState().state() == BluetoothState_State::BluetoothState_State_ON)));
      }
      else if(method_call.method_name() == "startScan"){
          ScanSettings scanSettings;
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
          ConnectedDevicesResponse response;
          auto p = bluetoothManager.getConnectedProtoBluetoothDevices();

          for(const auto& dev : p){
            *response.add_devices()=dev;
          }
          
          //[TODO] TEST THIS FUNCTION
          result->Success(flutter::EncodableValue(encodeToVector(response)));
      }
      else if(method_call.method_name() == "connect"){
        std::vector<uint8_t> encoded = std::get<std::vector<uint8_t>>(args);//to fix!
        ConnectRequest connectRequest;
        bool ok = connectRequest.ParseFromArray(encoded.data(), encoded.size());
        btlog::Logger::log(btlog::LogLevel::DEBUG, "size serialized = " + std::to_string(encoded.size()));
        bluetoothManager.connect(connectRequest);
        if(!ok)
          result->Error("could not deserialize request!");
        else
          result->Success(flutter::EncodableValue(NULL));
      }
      else if(method_call.method_name() == "disconnect"&&false){
        std::vector<uint8_t> encoded = std::get<std::vector<uint8_t>>(args);
        std::string deviceID(encoded.begin(), encoded.end());
        result->Success(flutter::EncodableValue(NULL));
      }
      else if(method_call.method_name() == "deviceState"){
        std::string deviceID = std::get<std::string>(args);
        std::scoped_lock lock(bluetoothManager.bluetoothDevices().mut);
        const auto& device=bluetoothManager.bluetoothDevices().var[deviceID];

        DeviceStateResponse res;
        res.set_remote_id(device.cAddress());
        res.set_state(fromLocalState(device.cState()));

        result->Success(flutter::EncodableValue(encodeToVector(res)));
      }
      else {
        result->NotImplemented();
      }
    }
  };

  static auto encodeToVector(const google::protobuf::MessageLite& messageLite) noexcept -> std::vector<u_int8_t>{
    std::vector<u_int8_t> encoded(messageLite.ByteSizeLong());
    messageLite.SerializeToArray(encoded.data(), messageLite.ByteSizeLong());
    return encoded;
  }
  static auto fromLocalState(const btu::BluetoothDeviceController::State& s) -> DeviceStateResponse_BluetoothDeviceState{
    using State=btu::BluetoothDeviceController::State;
    switch (s){
      case State::CONNECTED: return DeviceStateResponse_BluetoothDeviceState_CONNECTED;
      case State::CONNECTING: return DeviceStateResponse_BluetoothDeviceState_CONNECTING;
      case State::DISCONNECTED: return DeviceStateResponse_BluetoothDeviceState_DISCONNECTED;
      case State::DISCONNECTING: return DeviceStateResponse_BluetoothDeviceState_DISCONNECTING;
      default: return DeviceStateResponse_BluetoothDeviceState_DISCONNECTED;
    }
  }
}  // namespace

void FlutterBlueTizenPluginRegisterWithRegistrar(FlutterDesktopPluginRegistrarRef registrar) { 
      FlutterBlueTizenPlugin::RegisterWithRegistrar(flutter::PluginRegistrarManager::GetInstance()->GetRegistrar<flutter::PluginRegistrar>(registrar));
}

