#pragma once

#include <ArduinoJson.h>
#include <NTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <ESP8266HTTPClient.h>
#include <WiFiUdp.h>
#include "global_config.h"
#include "location_manager.h"

// Global variables and objects
extern WiFiUDP ntpUDP;
extern NTPClient timeClient;

extern time_t zoneStart;
extern time_t zoneEnd;
extern time_t timeNow;

extern int offset;
extern String city_name;

void getTimezone();
void printTimeToScreen();
void printDateToScreen();
void printDayToScreen();
void printCityToScreen();
void ntp_init();
