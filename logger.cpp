#include "logger.h"
#include "constants.h"

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

void Logger::begin(unsigned long baudRate) {
    if (!isInitialized) {
        Serial.begin(baudRate);
        isInitialized = true;
        delay(Timing::SERIAL_INIT_DELAY_MS); // Give time for Serial to initialize
    }
}

void Logger::setLogLevel(LogLevel level) {
    logLevel = level;
}

LogLevel Logger::getLogLevel() const {
    return logLevel;
}

const char* Logger::getLevelString(LogLevel level) {
    switch (level) {
        case LOG_LEVEL_ERROR:   return "[ERROR]  ";
        case LOG_LEVEL_WARNING: return "[WARN]   ";
        case LOG_LEVEL_INFO:    return "[INFO]   ";
        case LOG_LEVEL_DEBUG:   return "[DEBUG]  ";
        case LOG_LEVEL_VERBOSE: return "[VERBOSE]";
        default:                return "[UNKNOWN]";
    }
}

void Logger::log(LogLevel level, const String& message) {
    if (!isInitialized || !Serial || level > logLevel) {
        return;
    }
    
    // Print timestamp (milliseconds since start)
    unsigned long timestamp = millis();
    char timeStr[12];
    sprintf(timeStr, "%10lu", timestamp);
    
    Serial.print(timeStr);
    Serial.print(" ");
    Serial.print(getLevelString(level));
    Serial.print(" ");
    Serial.println(message);
}

void Logger::error(const String& message) {
    log(LOG_LEVEL_ERROR, message);
}

void Logger::warning(const String& message) {
    log(LOG_LEVEL_WARNING, message);
}

void Logger::info(const String& message) {
    log(LOG_LEVEL_INFO, message);
}

void Logger::debug(const String& message) {
    log(LOG_LEVEL_DEBUG, message);
}

void Logger::verbose(const String& message) {
    log(LOG_LEVEL_VERBOSE, message);
}

void Logger::error(const __FlashStringHelper* message) {
    error(String(message));
}

void Logger::warning(const __FlashStringHelper* message) {
    warning(String(message));
}

void Logger::info(const __FlashStringHelper* message) {
    info(String(message));
}

void Logger::debug(const __FlashStringHelper* message) {
    debug(String(message));
}

void Logger::verbose(const __FlashStringHelper* message) {
    verbose(String(message));
}

