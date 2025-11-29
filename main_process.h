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
#include <Ticker.h>
#include "MQTTClient.h"
#include "constants.h"

// TIME_TO_CALL_SERVICES moved to constants.h as Timing::DATA_UPDATE_INTERVAL_SEC

extern Ticker updateDataTicker;
extern bool isRunWeather;
//extern WeatherManager weatherManager;

// Function prototypes
void IRAM_ATTR runAllUpdates();
void setup();
void fetchWeatherAndCurrency();
void loop();
void enableWiFi();
void disableWiFi();
