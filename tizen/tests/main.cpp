#include <iostream>
#include <flutter.h>

class App : public FlutterApp {
 public:
  bool OnCreate() {
    if (FlutterApp::OnCreate()) {

    }
    return IsRunning();
  }
};

auto main(int argc, char *argv[]) -> int{
    App app;
    app.SetDartEntrypoint("serviceMain");
    
    std::cout<<"Running!\n";
    auto res=app.Run(argc, argv);

    return res;
}