#include <Logger.h>
#include <LogLevel.h>
namespace btlog{
    void Logger::log(LogLevel level, const std::string& mess) noexcept{
        // log_priority p;
        // switch (logLevel){
        //     case LogLevel::VERBOSE:
        //         p = log_priority::DLOG_VERBOSE;
        //         break;
        //     case LogLevel::DEBUG:
        //         p = log_priority::DLOG_DEBUG;
        //         break;
        //     case LogLevel::INFO:
        //         p = log_priority::DLOG_INFO;
        //         break;
        //     case LogLevel::WARNING:
        //         p = log_priority::DLOG_WARN;
        //         break;
        //     case LogLevel::ERROR:
        //         p = log_priority::DLOG_ERROR;
        //         break;
        //     case LogLevel::FATAL:
        //         p = log_priority::DLOG_FATAL;
        //         break;
        //     case LogLevel::SILENT:
        //         p = log_priority::DLOG_SILENT;
        //         break;
        //     default:
        //         p = log_priority::DLOG_DEBUG;
        //         break;
        // }
        // //change this!
        dlog_print(DLOG_ERROR, logTag.c_str(), mess.c_str(), "");
    }
    void Logger::setLogLevel(log_priority log_priority) noexcept{
        switch (log_priority){
            case DLOG_VERBOSE:
                logLevel = LogLevel::VERBOSE;
                break;
            case DLOG_DEBUG:
                logLevel = LogLevel::DEBUG;
                break;
            case DLOG_INFO:
                logLevel = LogLevel::INFO;
                break;
            case DLOG_WARN:
                logLevel = LogLevel::WARNING;
                break;
            case DLOG_ERROR:
                logLevel = LogLevel::ERROR;
                break;
            case DLOG_FATAL:
                logLevel = LogLevel::FATAL;
                break;
            case DLOG_SILENT:
                logLevel = LogLevel::SILENT;
                break;
            default:
                logLevel = LogLevel::DEBUG;
                break;
        }
    }
}