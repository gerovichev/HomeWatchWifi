#ifndef SECRET_H
#define SECRET_H

#include <Arduino.h>  // Include Arduino core for String and other Arduino types
#include <map>

// API Keys and URLs
extern const char* googleApiKey;
extern const String confPathCurrencyUSD;
extern const String confPathCurrencyEUR;
extern const String confBearerTokenCurrency;
extern const String appidWeather;
extern const String apiKeyTimezone;
extern const String webOTA_updateURL;

// MQTT broker credentials
extern const char* mqtt_server;
extern const char* mqtt_user;
extern const char* mqtt_password;
extern const String mqtt_topic;
extern String mqtt_topic_str;

// Device configuration structure
struct DeviceConfig {
    String lang_weather;
    String hostname_m;
    bool IS_DHT_CONNECTED;
    bool isWebClientNeeded;
    bool isReadWeather;
    double humidity_delta;
    String nameofWatch;
    bool isOTAreq;
    int intensity;
    bool isMQTT = false;
};

// Global device configuration map
extern std::map<String, DeviceConfig> configMap;

// Function declarations
void setDeviceConfig();

#endif // SECRET_H
