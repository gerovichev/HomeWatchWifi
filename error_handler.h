#pragma once

#include "logger.h"

/**
 * Централизованная обработка ошибок
 * Предоставляет единый интерфейс для обработки различных типов ошибок
 */
class ErrorHandler {
public:
    enum ErrorType {
        ERROR_NETWORK,
        ERROR_SENSOR,
        ERROR_API,
        ERROR_CONFIG,
        ERROR_CRITICAL
    };

    enum ErrorAction {
        ACTION_RETRY,
        ACTION_SKIP,
        ACTION_RESTART,
        ACTION_USE_CACHED
    };

    /**
     * Обработать ошибку с автоматическим выбором действия
     */
    static ErrorAction handleError(ErrorType type, const String& message, int attemptCount = 0, int maxAttempts = 3);

    /**
     * Логировать ошибку с соответствующим уровнем
     */
    static void logError(ErrorType type, const String& message);

    /**
     * Проверить, нужно ли повторить операцию
     */
    static bool shouldRetry(ErrorType type, int attemptCount, int maxAttempts);

private:
    static constexpr int MAX_RETRIES_NETWORK = 3;
    static constexpr int MAX_RETRIES_SENSOR = 2;
    static constexpr int MAX_RETRIES_API = 3;
};


