#ifndef NOTIFICATIONS_HANDLER_H
#define NOTIFICATIONS_HANDLER_H
#include <memory>
#include <Utils.h>

namespace btu{
    class NotificationsHandler{
        std::shared_ptr<MethodChannel> _methodChannel;
    
    public:
        NotificationsHandler(std::shared_ptr<MethodChannel>);
        auto notifyUIThread(const google::protobuf::Message&) noexcept -> void;
    };
}

#endif