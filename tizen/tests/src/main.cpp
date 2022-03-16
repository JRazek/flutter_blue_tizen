#include <iostream>
#include <flutter.h>
#include <Logger.h>
#include <thread>
#include <chrono>
#include <memory>

using namespace btlog;

class App : public FlutterApp {
 public:
  auto OnCreate() -> bool override{
    if (FlutterApp::OnCreate()){

    }
    return IsRunning();
  }
};

auto main(int argc, char *argv[]) -> int{
    std::shared_ptr<int> res=std::make_shared<int>(-1);
    std::thread run([](int argc, char *argv[], std::shared_ptr<int> res){
      App app;
      *res=app.Run(argc, argv);
    }, argc, argv, res);
    
    Logger::log(LogLevel::DEBUG, "Hello world!");//visible in app logs via sdb dlog.

    run.join();
    return *res;
}