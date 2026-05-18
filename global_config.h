#pragma once

#include <ArduinoOTA.h>
#include "secret.h"
#include "led_display.h"
#include "logger.h"

// Configuration-related global variables
extern String lang_weather;
// Bug fix #6: unsigned int is 16-bit on ESP8266 (max 65535). Unix epoch
// timestamps for sunrise/sunset are ~1.7 billion — they silently truncated,
// making day/night intensity switching always wrong. Use unsigned long (32-bit).
extern unsigned long sunrise;
extern unsigned long sunset;

extern String version_prg;
extern char grad;

extern float humidity_delta;
extern String hostname_m;
extern boolean isOTAreq;
extern boolean isMQTT;
extern String nameofWatch;
extern String macAddrSt;

extern String daysOfTheWeek[7];
extern boolean IS_DHT_CONNECTED;
extern bool isWebClientNeeded;
extern boolean isReadWeather;

void initPerDevice();
void verifyWifi();
String getNumberWithZerro(int dig);
void drawString(String tape);
char getGradValue();
