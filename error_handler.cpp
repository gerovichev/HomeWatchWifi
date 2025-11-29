#include "error_handler.h"

ErrorHandler::ErrorAction ErrorHandler::handleError(ErrorType type, const String& message, int attemptCount, int maxAttempts) {
    logError(type, message);

    if (shouldRetry(type, attemptCount, maxAttempts)) {
        return ACTION_RETRY;
    }

    switch (type) {
        case ERROR_NETWORK:
            // Network errors: try to use cached data if available
            return ACTION_USE_CACHED;
        
        case ERROR_SENSOR:
            // Sensor errors: skip this reading, continue
            return ACTION_SKIP;
        
        case ERROR_API:
            // API errors: use cached data if available, otherwise skip
            return ACTION_USE_CACHED;
        
        case ERROR_CONFIG:
            // Config errors: use defaults, continue
            return ACTION_SKIP;
        
        case ERROR_CRITICAL:
            // Critical errors: restart device
            LOG_ERROR_F("Critical error detected, restarting device...");
            delay(1000);  // Give time for log to flush
            ESP.restart();
            return ACTION_RESTART;
        
        default:
            return ACTION_SKIP;
    }
}

void ErrorHandler::logError(ErrorType type, const String& message) {
    switch (type) {
        case ERROR_NETWORK:
            LOG_WARNING("Network error: " + message);
            break;
        case ERROR_SENSOR:
            LOG_WARNING("Sensor error: " + message);
            break;
        case ERROR_API:
            LOG_ERROR("API error: " + message);
            break;
        case ERROR_CONFIG:
            LOG_ERROR("Config error: " + message);
            break;
        case ERROR_CRITICAL:
            LOG_ERROR("Critical error: " + message);
            break;
    }
}

bool ErrorHandler::shouldRetry(ErrorType type, int attemptCount, int maxAttempts) {
    if (attemptCount >= maxAttempts) {
        return false;
    }

    switch (type) {
        case ERROR_NETWORK:
            return attemptCount < MAX_RETRIES_NETWORK;
        case ERROR_SENSOR:
            return attemptCount < MAX_RETRIES_SENSOR;
        case ERROR_API:
            return attemptCount < MAX_RETRIES_API;
        default:
            return false;
    }
}


