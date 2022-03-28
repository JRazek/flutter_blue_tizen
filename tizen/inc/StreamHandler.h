#ifndef STREAM_HANDLER_H
#define STREAM_HANDLER_H

#include <flutter/event_stream_handler.h>

#include <functional>
#include <memory>

namespace btu{
    
class StreamHandler : public flutter::StreamHandler<>{
    using T=flutter::EncodableValue;
    using Base=flutter::StreamHandler<>;
    using err_type=flutter::StreamHandlerError<T>;
    virtual std::unique_ptr<flutter::StreamHandlerError<T>> OnListenInternal(
        const T* arguments,
        std::unique_ptr<flutter::EventSink<T>>&& events
    ) override{
        return nullptr;
    }

    virtual std::unique_ptr<flutter::StreamHandlerError<T>> OnCancelInternal(
        const T* arguments
    ) override{
        return nullptr;
    }
};

}
#endif //STREAM_HANDLER_H