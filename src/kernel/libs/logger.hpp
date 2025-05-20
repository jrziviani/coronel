#ifndef LOGGER_HPP
#define LOGGER_HPP

namespace lib
{
    enum class log_level : uint8_t
    {
        TRACE,
        INFO,
        WARNING,
        ERROR,
        CRITICAL
    };

    inline void log(log_level level, const char *message)
    {
        (void)level;
        (void)message;
    }
}

#endif // LOGGER_HPP