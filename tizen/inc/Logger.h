#ifndef LOGGER_H
#define LOGGER_h

#include <string>

#include <dlog.h>
#include <LogLevel.h>

namespace btlog{
    class Logger{
        inline static LogLevel logLevel;
        inline static std::string logTag = "FlutterBlueTizenPlugin";
    public:
        static void setLogLevel(log_priority log_priority) noexcept;
        static void log(LogLevel level, const std::string& mess) noexcept;
    };
} // namespace btlog

#endif //LOGGER_H