#ifndef SECRET_H
#define SECRET_H

#include <Arduino.h> // Include Arduino core for String and other Arduino types
#include <map>

// API Keys and URLs
extern const char *googleApiKey;
extern const char *confPathCurrencyUSD;
extern const char *confPathCurrencyEUR;
extern const char *confBearerTokenCurrency;
extern const char *appidWeather;
extern const char *apiKeyTimezone;
extern const char *webOTA_updateURL;
extern const char *confPathCryptoBTC;
extern const char *confBearerTokenCrypto;

// MQTT broker credentials
extern const char *mqtt_server;
extern const char *mqtt_user;
extern const char *mqtt_password;
extern const char *mqtt_topic;
extern String mqtt_topic_str;
extern const char *wifi_name;
extern const char *wifi_pass;

// Device configuration structure
struct DeviceConfig {
  String lang_weather;
  String hostname_m;
  bool IS_DHT_CONNECTED;
  bool isWebClientNeeded;
  bool isReadWeather;
  double humidity_delta;
  String nameofWatch;
  bool isOTAreq = true;
  int intensity;
  bool isMQTT = false;
};

// Global device configuration map
extern std::map<String, DeviceConfig> configMap;

// Function declarations
void setDeviceConfig();

#endif // SECRET_H
