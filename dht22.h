#pragma once

#include <DHT_U.h>
#include "global.h"


void drawString(String tape, int start);

#define DHTPIN 12      //0         // Pin which is connected to the DHT sensor. D6
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
    Serial.println("------------------------------------");
    Serial.println(type);
    Serial.print("Sensor:       ");
    Serial.println(sensor.name);
    Serial.print("Driver Ver:   ");
    Serial.println(sensor.version);
    Serial.print("Unique ID:    ");
    Serial.println(sensor.sensor_id);
    Serial.print("Max Value:    ");
    Serial.print(sensor.max_value);
    Serial.println(type == "Temperature" ? " *C" : " %");
    Serial.print("Min Value:    ");
    Serial.print(sensor.min_value);
    Serial.println(type == "Temperature" ? " *C" : " %");
    Serial.print("Resolution:   ");
    Serial.print(sensor.resolution);
    Serial.println(type == "Temperature" ? " *C" : " %");
    Serial.println("------------------------------------");
}

void readAndPrintTemperature() {
    sensors_event_t event;
    dht.temperature().getEvent(&event);
    if (isnan(event.temperature)) {
        handleTemperatureError();
    } else {
        homeTemp = event.temperature;
        Serial.print("Temperature: ");
        Serial.print(homeTemp);
        Serial.println(" *C");
        String tape = "T" + String(round(homeTemp), 0) + getGradValue() + "C";
        drawString(tape, 0);
    }
}

void readAndPrintHumidity() {
    sensors_event_t event;
    dht.humidity().getEvent(&event);
    if (isnan(event.relative_humidity)) {
        handleHumidityError();
    } else {
        homeHumidity = event.relative_humidity + humidity_delta;
        Serial.print("Humidity: ");
        Serial.print(homeHumidity);
        Serial.println("%");
        String tape = String(round(homeHumidity), 0) + "%";
        tape = tape.length() == 4 ? "H" + tape : "H " + tape;
        int mv = 5 - tape.length();
        drawString(tape, mv);
    }
}

void handleTemperatureError() {
    dht22Start();
    Serial.println("Error reading temperature!");
}

void handleHumidityError() {
    dht22Start();
    Serial.println("Error reading humidity!");
}
