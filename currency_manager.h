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
  // Using const char* directly to avoid String copies in RAM
  const char *bearerTokenCurrency;
  const char *pathCurrencyUSD;
  const char *pathCurrencyEUR;
  const char *pathCryptoBTC;

  float dataUSDValue;
  float dataEURValue;
  float dataBTCValue;

  // Response parser callback type
  typedef float (*ResponseParser)(const String& payload);

  // Unified HTTP fetch with retry logic — eliminates ~180 lines of duplication
  float fetchWithRetry(const char* path, const char* token, ResponseParser parser);

  // Convenience wrappers
  float readCurrency(const char* path);
  float readCrypto(const char* path);

  // Response parsers
  static float parseCurrencyResponse(const String& payload);
};
