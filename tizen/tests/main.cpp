#include <iostream>
#include <flutter.h>

class App : public FlutterApp {
 public:
  auto OnCreate() -> bool override{
    if (FlutterApp::OnCreate()){

    }
    return IsRunning();
  }
  auto test(int argc, char **argv) -> void{
    std::cout<<"Running!\n";
  }
};

auto main(int argc, char *argv[]) -> int{
    App app;
    app.SetDartEntrypoint("serviceMain");
    
    app.test(argc, argv);

    return app.Run(argc, argv);//this line crashes the program.
}