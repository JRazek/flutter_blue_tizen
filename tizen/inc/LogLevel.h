#ifndef LOG_LEVEL_H
#define LOG_LEVEL_H

namespace btlog{
    enum class LogLevel{
        VERBOSE = 0,
        DEBUG = 1,
        INFO = 2,
        WARNING = 3,
        ERROR = 4,
        FATAL = 5,
        SILENT = 6
    };
}
#endif //LOG_LEVEL_H