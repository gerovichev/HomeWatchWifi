#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>

// Log levels
enum LogLevel {
    LOG_LEVEL_NONE = 0,
    LOG_LEVEL_ERROR = 1,
    LOG_LEVEL_WARNING = 2,
    LOG_LEVEL_INFO = 3,
    LOG_LEVEL_DEBUG = 4,
    LOG_LEVEL_VERBOSE = 5
};

class Logger {
public:
    static Logger& getInstance();
    
    void begin(unsigned long baudRate = 115200);
    void setLogLevel(LogLevel level);
    LogLevel getLogLevel() const;
    
    void error(const String& message);
    void warning(const String& message);
    void info(const String& message);
    void debug(const String& message);
    void verbose(const String& message);
    
    void error(const __FlashStringHelper* message);
    void warning(const __FlashStringHelper* message);
    void info(const __FlashStringHelper* message);
    void debug(const __FlashStringHelper* message);
    void verbose(const __FlashStringHelper* message);

private:
    Logger() : logLevel(LOG_LEVEL_INFO), isInitialized(false) {}
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    
    void log(LogLevel level, const String& message);
    const char* getLevelString(LogLevel level);
    
    LogLevel logLevel;
    bool isInitialized;
};

// Convenience macros for logging
#define LOG_ERROR(msg) Logger::getInstance().error(msg)
#define LOG_WARNING(msg) Logger::getInstance().warning(msg)
#define LOG_INFO(msg) Logger::getInstance().info(msg)
#define LOG_DEBUG(msg) Logger::getInstance().debug(msg)
#define LOG_VERBOSE(msg) Logger::getInstance().verbose(msg)

// Macros with Flash strings
#define LOG_ERROR_F(msg) Logger::getInstance().error(F(msg))
#define LOG_WARNING_F(msg) Logger::getInstance().warning(F(msg))
#define LOG_INFO_F(msg) Logger::getInstance().info(F(msg))
#define LOG_DEBUG_F(msg) Logger::getInstance().debug(F(msg))
#define LOG_VERBOSE_F(msg) Logger::getInstance().verbose(F(msg))

#endif // LOGGER_H

