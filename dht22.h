#pragma once

#include <DHT_U.h>
#include "global.h"


void drawString(String tape, int start);

#define DHTPIN 12      //0         // Pin which is connected to the DHT sensor. D6
#define DHTTYPE DHT22  // DHT 22 (AM2302)

DHT_Unified dht(DHTPIN, DHTTYPE);

float homeTemp = 0.0;
float homeHumidity = 0.0;

void setHomeTemp();

void dht22Start() {
  if (IS_DHT_CONNECTED) {
    dht.begin();
    sensor_t sensor;
    dht.temperature().getSensor(&sensor);
    Serial.println("------------------------------------");
    Serial.println("Temperature");
    Serial.print("Sensor:       ");
    Serial.println(sensor.name);
    Serial.print("Driver Ver:   ");
    Serial.println(sensor.version);
    Serial.print("Unique ID:    ");
    Serial.println(sensor.sensor_id);
    Serial.print("Max Value:    ");
    Serial.print(sensor.max_value);
    Serial.println(" *C");
    Serial.print("Min Value:    ");
    Serial.print(sensor.min_value);
    Serial.println(" *C");
    Serial.print("Resolution:   ");
    Serial.print(sensor.resolution);
    Serial.println(" *C");
    Serial.println("------------------------------------");
    // Print humidity sensor details.
    dht.humidity().getSensor(&sensor);
    Serial.println("------------------------------------");
    Serial.println("Humidity");
    Serial.print("Sensor:       ");
    Serial.println(sensor.name);
    Serial.print("Driver Ver:   ");
    Serial.println(sensor.version);
    Serial.print("Unique ID:    ");
    Serial.println(sensor.sensor_id);
    Serial.print("Max Value:    ");
    Serial.print(sensor.max_value);
    Serial.println("%");
    Serial.print("Min Value:    ");
    Serial.print(sensor.min_value);
    Serial.println("%");
    Serial.print("Resolution:   ");
    Serial.print(sensor.resolution);
    Serial.println("%");
    Serial.println("------------------------------------");
    setHomeTemp();
  }
}

void setHomeTemp() {
  sensors_event_t event;
  dht.temperature().getEvent(&event);
  if (isnan(event.temperature)) {
    dht22Start();
    //drawString("T error", 0);
    Serial.println("Error reading temperature!");
  } else {
    homeTemp = event.temperature;
  }
}

void printHomeTemp() {
  sensors_event_t event;
  dht.temperature().getEvent(&event);
  if (isnan(event.temperature)) {
    dht22Start();
    //drawString("T error", 0);
    Serial.println("Error reading temperature!");
  } else {

    homeTemp = event.temperature;
    Serial.print("Temperature: ");
    Serial.print(homeTemp);
    Serial.println(" *C");

    String tape = "T" + String(round(homeTemp), 0) + getGradValue() + "C";  //247
    drawString(tape, 0);
  }
}

void printHumidity() {
  sensors_event_t event;

  // Get humidity event and print its value.
  dht.humidity().getEvent(&event);
  if (isnan(event.relative_humidity)) {
    //drawString("H error", 0);
    dht22Start();
    //Serial.println("Error reading humidity!");
  } else {
    homeHumidity = event.relative_humidity + humidity_delta;
    Serial.print("Humidity: ");
    Serial.print(homeHumidity);
    Serial.println("%");
    String tape = String(round(homeHumidity), 0) + "%";
    if (tape.length() == 4) {
      tape = "H" + tape;
    } else {
      tape = "H " + tape;
    }
    int mv = 5 - tape.length();
    drawString(tape, mv);
  }
}
