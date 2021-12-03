#include <Logger.h>
#include <LogLevel.h>

namespace btlog{
    void Logger::log(LogLevel messLevel, const std::string& mess) noexcept{
        if(messLevel <= logLevel.var){
            log_priority p;
            switch (messLevel){
                case LogLevel::EMERGENCY:
                    p = log_priority::DLOG_FATAL;
                    break;
                case LogLevel::ALERT:
                    p = log_priority::DLOG_FATAL;
                    break;
                case LogLevel::CRITICAL:
                    p = log_priority::DLOG_FATAL;
                    break;
                case LogLevel::ERROR:
                    p = log_priority::DLOG_ERROR;
                    break;
                case LogLevel::WARNING:
                    p = log_priority::DLOG_WARN;
                    break;
                case LogLevel::NOTICE:
                    p = log_priority::DLOG_INFO;
                    break;
                case LogLevel::INFO:
                    p = log_priority::DLOG_INFO;
                    break;
                case LogLevel::DEBUG:
                    p = log_priority::DLOG_DEBUG;
                    break;
                default:
                    p = log_priority::DLOG_VERBOSE;
                    break;
            }
            std::scoped_lock l(m);
            dlog_print(p, logTag.c_str(), mess.c_str(), "");
        }
    }
    void Logger::setLogLevel(LogLevel _logLevel) noexcept{
        std::scoped_lock lock(logLevel.mut);
        logLevel.var = _logLevel;
        log(LogLevel::DEBUG, "set log level to =  " + std::to_string(static_cast<int>(logLevel.var)));
    }
}