#ifndef STATE_HANDLER_H
#define STATE_HANDLER_H

#include <flutter/event_stream_handler.h>

#include <functional>
#include <memory>

namespace btu{
    
class StateHandler : public flutter::StreamHandler<>{
    using T=flutter::EncodableValue;
    using Base=flutter::StreamHandler<>;
    using err_type=flutter::StreamHandlerError<T>;

    virtual auto OnListenInternal(
        const T* arguments,
        std::unique_ptr<flutter::EventSink<T>>&& events
    ) -> std::unique_ptr<flutter::StreamHandlerError<T>> override;

    virtual auto OnCancelInternal(
        const T* arguments
    ) -> std::unique_ptr<flutter::StreamHandlerError<T>> override;
};

}
#endif //STATE_HANDLER_H