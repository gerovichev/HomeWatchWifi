// CurrencyManager.cpp
#include "currency_manager.h"

CurrencyManager::CurrencyManager()
    : bearerTokenCurrency(confBearerTokenCurrency), pathCurrencyUSD(confPathCurrencyUSD), pathCurrencyEUR(confPathCurrencyEUR),
      dataUSDValue(0.0), dataEURValue(0.0) {}

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

bool CurrencyManager::setupHttpClient(HTTPClient &http, BearSSL::WiFiClientSecure &client, const String &path) {
    client.setInsecure(); // Disable SSL certificate verification
    http.setTimeout(HTTP_TIMEOUT);

    if (Serial) Serial.println(path);

    if (!http.begin(client, path)) {
        if (Serial) Serial.println(F("Failed to begin HTTP connection"));
        return false;
    }

    http.addHeader(F("Authorization"), F("Bearer ") + bearerTokenCurrency);
    http.addHeader(F("Content-Type"), F("application/json"));
    return true;
}

float CurrencyManager::handleHttpResponse(HTTPClient &http) {
    int httpCode = http.GET();
    if (httpCode != HTTP_CODE_OK) {
        if (Serial) Serial.println(F("HTTP request failed with code: ") + String(httpCode, DEC));
        return 0.0;
    }

    String payload = http.getString();
    if (Serial) {
        Serial.println(F("Payload: "));
        Serial.println(payload);
        Serial.println(F("Payload length: "));
        Serial.println(payload.length());
    }

    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, payload);
    if (error) {
        if (Serial) {
            Serial.print(F("deserializeJson() failed: "));
            Serial.println(error.f_str());
        }
        return 0.0;
    }

    return doc[F("state")]; // Extract currency value from JSON
}

float CurrencyManager::readCurrency(const String &path) {
    if (Serial) Serial.println(F("Start getting currency"));

    BearSSL::WiFiClientSecure client;
    HTTPClient http;

    if (Serial) Serial.println(path);

    if (!setupHttpClient(http, client, path)) {
        return 0.0;
    }

    float currencyValue = handleHttpResponse(http);
    http.end();
    return currencyValue;
}

