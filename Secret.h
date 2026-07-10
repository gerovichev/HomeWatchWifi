#ifndef SECRET_H
#define SECRET_H

#include <Arduino.h> // Include Arduino core for String and other Arduino types

// API Keys and URLs
extern const char *googleApiKey;
extern const char *confPathCurrencyUSD;
extern const char *confPathCurrencyEUR;
extern const char *confBearerTokenCurrency;
extern const char *appidWeather;
extern const char *apiKeyTimezone;
extern const char *webOTA_updateURL;
extern const char *confPathCryptoBTC;

// MQTT broker credentials
extern const char *mqtt_server;
extern const char *mqtt_user;
extern const char *mqtt_password;
extern const char *mqtt_topic;
extern String mqtt_topic_str;
extern const char *wifi_name;
extern const char *wifi_pass;

// Device configuration structure — uses const char* to avoid heap String allocations
struct DeviceConfig {
  const char* lang_weather;
  const char* hostname_m;
  bool IS_DHT_CONNECTED;
  bool isWebClientNeeded;
  bool isReadWeather;
  double humidity_delta;
  const char* nameofWatch;
  bool isOTAreq;
  int intensity;
  bool isMQTT;
};

// Number of device configurations
extern const int DEVICE_CONFIG_COUNT;

// Device configurations stored as a flat array (avoids std::map heap overhead)
struct DeviceConfigEntry {
  const char* mac;
  DeviceConfig config;
};

extern const DeviceConfigEntry deviceConfigs[];

// Function to find config by MAC address, returns nullptr if not found
const DeviceConfig* findDeviceConfig(const String& mac);

// Function declarations
void setDeviceConfig();

#endif // SECRET_H
