#include <StateHandler.h>

namespace btu{


auto StateHandler::OnListenInternal( const T* arguments, std::unique_ptr<flutter::EventSink<T>>&& events) -> std::unique_ptr<flutter::StreamHandlerError<T>>{
    return nullptr;
}

auto StateHandler::OnCancelInternal(const T* arguments) -> std::unique_ptr<flutter::StreamHandlerError<T>>{
    return nullptr;
}

}