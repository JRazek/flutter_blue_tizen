#include <NotificationsHandler.h>


namespace btu{
    NotificationsHandler::NotificationsHandler(std::shared_ptr<MethodChannel> methodChannel):
    _methodChannel(methodChannel)
    {}
    
    auto NotificationsHandler::notifyUIThread(const google::protobuf::Message& mess) noexcept -> void{

    }
}
