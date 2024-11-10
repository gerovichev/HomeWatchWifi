#pragma once

#include <DHT_U.h>
#include "global_config.h"

// Pin and sensor type definitions
#define DHTPIN 12      // Pin which is connected to the DHT sensor.
#define DHTTYPE DHT22  // DHT 22 (AM2302)


class Dht22_manager : public DHT_Unified
{
public:
  Dht22_manager();
  void printHomeTemp();
  void printHumidity();
  void dht22Start();
  float getHomeTemp();

private:
  // Global variables for home temperature and humidity
  float homeTemp;
  float homeHumidity;

  // Function prototypes

  void setHomeTemp();


  void printSensorDetails(sensor_t sensor, const char* type);
  void readAndPrintTemperature();
  void readAndPrintHumidity();
  void handleTemperatureError();
  void handleHumidityError();
};
