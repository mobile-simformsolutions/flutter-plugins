#include "include/desktop_multi_window/desktop_multi_window_plugin.h"
#include "multi_window_plugin_internal.h"

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>
#include <flutter/standard_method_codec.h>

#include <map>
#include <memory>

#include "multi_window_manager.h"
#include "screen_manager.cpp"

namespace {

class DesktopMultiWindowPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrarWindows *registrar);

  DesktopMultiWindowPlugin();

  ~DesktopMultiWindowPlugin() override;

 private:
  ScreenManager *screen_manager;
  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);
};

HWND screenView;

// static
void DesktopMultiWindowPlugin::RegisterWithRegistrar(
    flutter::PluginRegistrarWindows *registrar) {
  auto channel =
      std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
          registrar->messenger(), "mixin.one/flutter_multi_window",
          &flutter::StandardMethodCodec::GetInstance());

  auto plugin = std::make_unique<DesktopMultiWindowPlugin>();

  channel->SetMethodCallHandler(
      [plugin_pointer = plugin.get()](const auto &call, auto result) {
        plugin_pointer->HandleMethodCall(call, std::move(result));
      });

  registrar->AddPlugin(std::move(plugin));
  screenView = registrar->GetView()->GetNativeWindow();
}

DesktopMultiWindowPlugin::DesktopMultiWindowPlugin()  {
  screen_manager = new ScreenManager();
}

DesktopMultiWindowPlugin::~DesktopMultiWindowPlugin() = default;

void DesktopMultiWindowPlugin::HandleMethodCall(
    const flutter::MethodCall<flutter::EncodableValue> &method_call,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  if (method_call.method_name() == "createWindow") {
    auto args = std::get_if<std::string>(method_call.arguments());
    auto window_id = MultiWindowManager::Instance()->Create(args != nullptr ? *args : "");
    result->Success(flutter::EncodableValue(window_id));
    return;
  } else if (method_call.method_name() == "show") {
    auto window_id = method_call.arguments()->LongValue();
    MultiWindowManager::Instance()->Show(window_id);
    result->Success();
    return;
  } else if (method_call.method_name() == "hide") {
    auto window_id = method_call.arguments()->LongValue();
    MultiWindowManager::Instance()->Hide(window_id);
    result->Success();
    return;
  } else if (method_call.method_name() == "close") {
    auto window_id = method_call.arguments()->LongValue();
    MultiWindowManager::Instance()->Close(window_id);
    result->Success();
    return;
  } else if (method_call.method_name() == "setFrame") {
    auto *arguments = std::get_if<flutter::EncodableMap>(method_call.arguments());
    auto window_id = arguments->at(flutter::EncodableValue("windowId")).LongValue();
    auto left = std::get<double_t>(arguments->at(flutter::EncodableValue("left")));
    auto top = std::get<double_t>(arguments->at(flutter::EncodableValue("top")));
    auto width = std::get<double_t>(arguments->at(flutter::EncodableValue("width")));
    auto height = std::get<double_t>(arguments->at(flutter::EncodableValue("height")));
    MultiWindowManager::Instance()->SetFrame(window_id, left, top, width, height);
    result->Success();
    return;
  } else if (method_call.method_name() == "center") {
    auto window_id = method_call.arguments()->LongValue();
    MultiWindowManager::Instance()->Center(window_id);
    result->Success();
    return;
  } else if (method_call.method_name() == "setTitle") {
    auto *arguments = std::get_if<flutter::EncodableMap>(method_call.arguments());
    auto window_id = arguments->at(flutter::EncodableValue("windowId")).LongValue();
    auto title = std::get<std::string>(arguments->at(flutter::EncodableValue("title")));
    MultiWindowManager::Instance()->SetTitle(window_id, title);
    result->Success();
    return;
  } else if (method_call.method_name() == "getAllSubWindowIds") {
    auto window_ids = MultiWindowManager::Instance()->GetAllSubWindowIds();
    result->Success(flutter::EncodableValue(window_ids));
    return;
  } else if ((method_call.method_name().compare("getAttachedScreenList")) == 0) {
    result->Success(screen_manager->GetAttachedScreenList());
    return;
  }  else if ((method_call.method_name().compare("ensureScreenInitialized")) == 0) {
    screen_manager->native_window =
            ::GetAncestor(screenView, GA_ROOT);
    result->Success(flutter::EncodableValue(true));
    return;
  } else if ((method_call.method_name().compare("setFullScreen")) == 0) {
    const flutter::EncodableMap &args =
            std::get<flutter::EncodableMap>(*method_call.arguments());
    screen_manager->SetFullScreen(args);
    result->Success(flutter::EncodableValue(true));
    return;
  } else if ((method_call.method_name().compare("getBounds")) == 0) {
    const flutter::EncodableMap& args =
            std::get<flutter::EncodableMap>(*method_call.arguments());
    flutter::EncodableMap value = screen_manager->GetBounds(args);
    result->Success(flutter::EncodableValue(value));
    return;
  } else if ((method_call.method_name().compare("setBounds")) == 0) {
    const flutter::EncodableMap& args =
            std::get<flutter::EncodableMap>(*method_call.arguments());
    screen_manager->SetBounds(args);
    result->Success(flutter::EncodableValue(true));
    return;
  } else if ((method_call.method_name().compare("waitUntilReadyToShow")) == 0) {
    screen_manager->WaitUntilReadyToShow();
    result->Success(flutter::EncodableValue(true));
    return;
  } else if ((method_call.method_name().compare("setResizable")) == 0) {
    const flutter::EncodableMap& args =
            std::get<flutter::EncodableMap>(*method_call.arguments());
    screen_manager->SetResizable(args);
    result->Success(flutter::EncodableValue(true));
    return;
  } else if ((method_call.method_name().compare("setAlwaysOnTop")) == 0) {
    const flutter::EncodableMap& args =
            std::get<flutter::EncodableMap>(*method_call.arguments());
    screen_manager->SetAlwaysOnTop(args);
    result->Success(flutter::EncodableValue(true));
    return;
  } else if ((method_call.method_name().compare("setSkipTaskbar")) == 0) {
    const flutter::EncodableMap& args =
            std::get<flutter::EncodableMap>(*method_call.arguments());
    screen_manager->SetSkipTaskbar(args);
    result->Success(flutter::EncodableValue(true));
    return;
  } else if ((method_call.method_name().compare("isClosable")) == 0) {
    bool value = screen_manager->IsClosable();
    result->Success(flutter::EncodableValue(value));
    return;
  } else if ((method_call.method_name().compare("setClosable")) == 0) {
    const flutter::EncodableMap& args =
            std::get<flutter::EncodableMap>(*method_call.arguments());
    screen_manager->SetClosable(args);
    result->Success(flutter::EncodableValue(true));
    return;
  } else if ((method_call.method_name().compare("destroy")) == 0) {
    screen_manager->Destroy();
    result->Success(flutter::EncodableValue(true));
    return;
  }
  result->NotImplemented();
}

}  // namespace

void DesktopMultiWindowPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {

  InternalMultiWindowPluginRegisterWithRegistrar(registrar);

  // Attach MainWindow for
  auto hwnd = FlutterDesktopViewGetHWND(FlutterDesktopPluginRegistrarGetView(registrar));
  auto channel = WindowChannel::RegisterWithRegistrar(registrar, 0);
  MultiWindowManager::Instance()->AttachFlutterMainWindow(GetAncestor(hwnd, GA_ROOT),
                                                          std::move(channel));
}

void InternalMultiWindowPluginRegisterWithRegistrar(FlutterDesktopPluginRegistrarRef registrar) {
  DesktopMultiWindowPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrarWindows>(registrar));
}
