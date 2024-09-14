#pragma once

#include <DHT_U.h>
#include "global_config.h"

// Pin and sensor type definitions
#define DHTPIN 12      // Pin which is connected to the DHT sensor.
#define DHTTYPE DHT22  // DHT 22 (AM2302)

// DHT object
extern DHT_Unified dht;

// Global variables for home temperature and humidity
extern float homeTemp;
extern float homeHumidity;

// Function prototypes
void dht22Start();
void setHomeTemp();
void printHomeTemp();
void printHumidity();
void printSensorDetails(sensor_t sensor, const char* type);
void readAndPrintTemperature();
void readAndPrintHumidity();
void handleTemperatureError();
void handleHumidityError();
