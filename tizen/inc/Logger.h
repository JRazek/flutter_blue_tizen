#ifndef LOGGER_H
#define LOGGER_H

#include <string>

#include <dlog.h>
#include <LogLevel.h>
#include <Utils.h>

namespace btlog{
    class Logger{
        inline static btu::SafeType<LogLevel> logLevel{LogLevel::DEBUG};
        inline static std::string logTag = "FlutterBlueTizenPlugin";
        inline static std::mutex m;
    public:
        static void setLogLevel(LogLevel _logLevel) noexcept;
        static void log(LogLevel level, const std::string& mess) noexcept;
    };
} // namespace btlog

#endif //LOGGER_H