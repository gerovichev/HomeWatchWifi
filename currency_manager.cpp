// CurrencyManager.cpp
#include "currency_manager.h"
#include "secure_client.h"
#include "constants.h"
#include "logger.h"

CurrencyManager::CurrencyManager()
    : bearerTokenCurrency(confBearerTokenCurrency), pathCurrencyUSD(confPathCurrencyUSD), pathCurrencyEUR(confPathCurrencyEUR),
      dataUSDValue(0.0), dataEURValue(0.0) 
{
    // const char* pointers are assigned directly - no String copies, saves RAM
}

void CurrencyManager::initialize() {
    if (float tmpDataUSD = readCurrency(pathCurrencyUSD); tmpDataUSD > 0) {
        dataUSDValue = tmpDataUSD;
    }

    yield(); // Allow system tasks

    if (float tmpDataEUR = readCurrency(pathCurrencyEUR); tmpDataEUR > 0) {
        dataEURValue = tmpDataEUR;
    }
}

void CurrencyManager::displayUSDToScreen() {
    if (dataUSDValue > 0) {
        drawString(F("$ ") + String(dataUSDValue, 2));
    }
}

void CurrencyManager::displayEURToScreen() {
    if (dataEURValue > 0) {
        drawString(F("\x84 ") + String(dataEURValue, 2));
    }
}

bool CurrencyManager::setupHttpClient(HTTPClient &http, BearSSL::WiFiClientSecure &client, const char* path) {
    setupSecureClient(client, "currency API");
    http.setTimeout(Timing::HTTP_TIMEOUT_CURRENCY_MS);

    LOG_DEBUG("Currency API path: " + String(path));

    if (!http.begin(client, path)) {
        LOG_ERROR_F("Failed to begin HTTP connection");
        return false;
    }

    String authHeader = F("Bearer ") + String(bearerTokenCurrency);
    http.addHeader(F("Authorization"), authHeader);
    http.addHeader(F("Content-Type"), F("application/json"));
    return true;
}

float CurrencyManager::handleHttpResponse(HTTPClient &http) {
    int httpCode = http.GET();
    if (httpCode != HTTP_CODE_OK) {
        LOG_WARNING("HTTP request failed with code: " + String(httpCode, DEC));
        return 0.0;
    }

    String payload = http.getString();
    LOG_VERBOSE("Currency API payload length: " + String(payload.length()));

    StaticJsonDocument<Buffer::JSON_CURRENCY_SIZE> doc;
    DeserializationError error = deserializeJson(doc, payload);
    if (error) {
        LOG_ERROR("deserializeJson() failed: " + String(error.c_str()));
        return 0.0;
    }

    return doc[F("state")]; // Extract currency value from JSON
}

float CurrencyManager::readCurrency(const char* path) {
    LOG_DEBUG_F("Fetching currency data...");

    BearSSL::WiFiClientSecure client;
    HTTPClient http;

    if (!setupHttpClient(http, client, path)) {
        return 0.0;
    }

    float currencyValue = handleHttpResponse(http);
    http.end();
    return currencyValue;
}

