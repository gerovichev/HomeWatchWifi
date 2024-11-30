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
    static constexpr int HTTP_TIMEOUT = 1500; // Timeout for HTTP requests in milliseconds

    String bearerTokenCurrency;
    String pathCurrencyUSD;
    String pathCurrencyEUR;

    float dataUSDValue;
    float dataEURValue;

    bool setupHttpClient(HTTPClient &http, BearSSL::WiFiClientSecure &client, const String &path);
    float handleHttpResponse(HTTPClient &http);
    float readCurrency(const String &path);
};

