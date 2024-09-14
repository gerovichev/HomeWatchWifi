#pragma once

#include "global_config.h"
#include "led_display.h"
#include "WiFiSetup.h"
#include "location_manager.h"
#include "TimeManager.h"
#include "dht22_manager.h"
#include "weather_manager.h"
#include "OTAUpdate.h"
#include "clock.h"
#include "web_server.h"
#include <Ticker.h>
#include "MQTTClient.h"

constexpr int TIME_TO_CALL_SERVICES = 900;  // in seconds

extern Ticker updateDataTicker;
extern bool isRunWeather;

// Function prototypes
void IRAM_ATTR runAllUpdates();
void setup();
void fetchWeatherAndCurrency();
void loop();
