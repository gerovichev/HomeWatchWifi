#pragma once

#include <DHT_U.h>
#include "global.h"

#define DHTPIN 12      // Pin which is connected to the DHT sensor.
#define DHTTYPE DHT22  // DHT 22 (AM2302)

DHT_Unified dht(DHTPIN, DHTTYPE);

float homeTemp = 0.0;
float homeHumidity = 0.0;

// Function prototypes
void setHomeTemp();
void printSensorDetails(sensor_t sensor, const char* type);
void readAndPrintTemperature();
void readAndPrintHumidity();
void handleTemperatureError();
void handleHumidityError();

void dht22Start() {
    if (IS_DHT_CONNECTED) {
        dht.begin();
        sensor_t sensor;
        
        // Print temperature sensor details.
        dht.temperature().getSensor(&sensor);
        printSensorDetails(sensor, "Temperature");
        
        // Print humidity sensor details.
        dht.humidity().getSensor(&sensor);
        printSensorDetails(sensor, "Humidity");
        
        setHomeTemp();
    }
}

void setHomeTemp() {
    sensors_event_t event;
    dht.temperature().getEvent(&event);
    if (isnan(event.temperature)) {
        handleTemperatureError();
    } else {
        homeTemp = event.temperature;
    }
}

void printHomeTemp() {
    readAndPrintTemperature();
}

void printHumidity() {
    readAndPrintHumidity();
}

void printSensorDetails(sensor_t sensor, const char* type) {
    if (Serial) {
      Serial.println(F("------------------------------------"));
      Serial.println(type);
      
      Serial.print(F("Sensor:       "));
      Serial.println(sensor.name);
      
      Serial.print(F("Driver Ver:   "));
      Serial.println(sensor.version);
      
      Serial.print(F("Unique ID:    "));
      Serial.println(sensor.sensor_id);
      
      // Determine the unit based on the sensor type
      const char* unit = (strcmp(type, "Temperature") == 0) ? " *C" : " %";
      
      Serial.print(F("Max Value:    "));
      Serial.print(sensor.max_value);
      Serial.println(unit);
      
      Serial.print(F("Min Value:    "));
      Serial.print(sensor.min_value);
      Serial.println(unit);
      
      Serial.print(F("Resolution:   "));
      Serial.print(sensor.resolution);
      Serial.println(unit);
      
      Serial.println(F("------------------------------------"));
    }
}

void readAndPrintTemperature() {
    sensors_event_t event;
    dht.temperature().getEvent(&event);
    if (isnan(event.temperature)) {
        handleTemperatureError();
    } else {
        homeTemp = event.temperature;
        if (Serial) Serial.print(F("Temperature: "));
        if (Serial) Serial.print(homeTemp);
        if (Serial) Serial.println(F(" *C"));
        String tape = F("T") + String(round(homeTemp), 0) + getGradValue() + "C";
        drawString(tape);
    }
}

void readAndPrintHumidity() {
    sensors_event_t event;
    dht.humidity().getEvent(&event);
    if (isnan(event.relative_humidity)) {
        handleHumidityError();
    } else {
        homeHumidity = event.relative_humidity + humidity_delta;
        if (Serial) Serial.print(F("Humidity: "));
        if (Serial) Serial.print(homeHumidity);
        if (Serial) Serial.println(F("%"));
        String tape = String(round(homeHumidity), 0) + "%";
        tape = tape.length() == 4 ? F("H") + tape : F("H ") + tape;
        drawString(tape);
    }
}

void handleTemperatureError() {
    dht22Start();
    if (Serial) Serial.println(F("Error reading temperature!"));
}

void handleHumidityError() {
    dht22Start();
    if (Serial) Serial.println(F("Error reading humidity!"));
}
