#pragma once

#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>
#include "secret.h"
#include "led_display.h"

// Configuration-related global variables
extern String lang_weather;
extern unsigned int sunrise;
extern unsigned int sunset;

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
