// CurrencyManager.h
#pragma once

#include "global_config.h"
#include <WiFiClientSecureBearSSL.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>

class CurrencyManager {
public:
    CurrencyManager();
    
    void initialize();
    void displayUSDToScreen();
    void displayEURToScreen();

private:
    // HTTP_TIMEOUT moved to constants.h as Timing::HTTP_TIMEOUT_CURRENCY_MS

    // Using const char* directly to avoid String copies in RAM
    const char* bearerTokenCurrency;
    const char* pathCurrencyUSD;
    const char* pathCurrencyEUR;

    float dataUSDValue;
    float dataEURValue;

    bool setupHttpClient(HTTPClient &http, BearSSL::WiFiClientSecure &client, const char* path);
    float handleHttpResponse(HTTPClient &http);
    float readCurrency(const char* path);
};

