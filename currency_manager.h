// CurrencyManager.h
#pragma once

#include "global_config.h"
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>

class CurrencyManager {
public:
  CurrencyManager();

  void initialize();
  void displayUSDToScreen();
  void displayEURToScreen();
  void displayETHToScreen(); // Future use
  void displayBTCToScreen();

private:
  // HTTP_TIMEOUT moved to constants.h as Timing::HTTP_TIMEOUT_CURRENCY_MS

  // Using const char* directly to avoid String copies in RAM
  const char *bearerTokenCurrency;
  const char *bearerTokenCrypto;
  const char *pathCurrencyUSD;
  const char *pathCurrencyEUR;
  const char *pathCryptoBTC;

  float dataUSDValue;
  float dataEURValue;
  float dataBTCValue;

  bool setupHttpClient(HTTPClient &http, BearSSL::WiFiClientSecure &client,
                       const char *path, const char *token);
  float handleHttpResponse(HTTPClient &http);
  float handleCryptoResponse(HTTPClient &http);
  float readCurrency(const char *path);
  float readCrypto(const char *path);
};
