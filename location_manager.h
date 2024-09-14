#pragma once

#include <WifiLocation.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include <ESP8266HTTPClient.h>
#include <BearSSLHelpers.h>
#include "global_config.h"

// Structure to store configuration
struct Config {
  float latitude;
  float longitude;
  String ip;
};

// External variables
extern String ip;
extern float latitude;
extern float longitude;
extern Config config;

// Function prototypes
void loadConfiguration();
void saveConfiguration();
void setClock();
void getLocationAPI(String ip);
String getIp();
void location_init();
