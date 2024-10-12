#pragma once

#include "global_config.h"
#include "location_manager.h"

// Global variables to store weather data
extern unsigned int temperature;
extern unsigned int temp_max;
extern unsigned int pressure;
extern unsigned int main_ext_humidity;
extern String description_weather;

// Function prototypes
void readWeather();
void printWeatherToScreen();
void printMaxTempToScreen();
void printPressureToScreen();
void printHumidityToScreen();
void printDescriptionWeatherToScreen();
