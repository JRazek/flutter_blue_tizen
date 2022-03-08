#include <iostream>
#include <flutter.h>

class App : public FlutterApp {
 public:
  bool OnCreate() {
    if (FlutterApp::OnCreate()) {

    }
    return IsRunning();
  }
  auto test(int argc, char **argv) -> void{
    std::cout<<"Running!\n";
  }
  auto Run(int argc, char **argv) -> int override{
    std::cout<<"Hello!!!\n";
  }
};

auto main(int argc, char *argv[]) -> int{
    App app;
    app.SetDartEntrypoint("serviceMain");
    
    app.test(argc, argv);

    return app.Run(argc, argv);//this line crashes the program.
}