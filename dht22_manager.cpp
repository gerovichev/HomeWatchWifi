#include "dht22_manager.h"

// Define global variables
float homeTemp = 0.0;
float homeHumidity = 0.0;

Dht22_manager::Dht22_manager() : DHT_Unified(DHTPIN, DHTTYPE){}

// Function to initialize the DHT22 sensor and set home temperature
void Dht22_manager::dht22Start() {
    if (IS_DHT_CONNECTED) {
        begin();
        sensor_t sensor;

        // Print temperature sensor details
        temperature().getSensor(&sensor);
        printSensorDetails(sensor, "Temperature");

        // Print humidity sensor details
        humidity().getSensor(&sensor);
        printSensorDetails(sensor, "Humidity");

        setHomeTemp();  // Read and set initial home temperature
    }
}

// Function to read and set the home temperature
void Dht22_manager::setHomeTemp() {
    sensors_event_t event;
    temperature().getEvent(&event);
    if (isnan(event.temperature)) {
        handleTemperatureError();
    } else {
        homeTemp = event.temperature;
    }
}

float Dht22_manager::getHomeTemp()
{
  return homeTemp;
}

// Function to print home temperature to the display
void Dht22_manager::printHomeTemp() {
    readAndPrintTemperature();
}

// Function to print humidity to the display
void Dht22_manager::printHumidity() {
    readAndPrintHumidity();
}

// Function to print detailed sensor information
void Dht22_manager::printSensorDetails(sensor_t sensor, const char* type) {
    if (Serial) {
        Serial.println(F("------------------------------------"));
        Serial.println(type);

        Serial.print(F("Sensor:       "));
        Serial.println(sensor.name);

        Serial.print(F("Driver Ver:   "));
        Serial.println(sensor.version);

        Serial.print(F("Unique ID:    "));
        Serial.println(sensor.sensor_id);

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

// Function to read and print temperature to the display
void Dht22_manager::readAndPrintTemperature() {
    sensors_event_t event;
    temperature().getEvent(&event);
    if (isnan(event.temperature)) {
        handleTemperatureError();
    } else {
        homeTemp = event.temperature;
        if (Serial) {
            Serial.print(F("Temperature: "));
            Serial.print(homeTemp);
            Serial.println(F(" *C"));
        }
        String tape = F("T") + String(round(homeTemp), 0) + getGradValue() + "C";
        drawString(tape);
    }
}

// Function to read and print humidity to the display
void Dht22_manager::readAndPrintHumidity() {
    sensors_event_t event;
    humidity().getEvent(&event);
    if (isnan(event.relative_humidity)) {
        handleHumidityError();
    } else {
        homeHumidity = event.relative_humidity + humidity_delta;
        if (Serial) {
            Serial.print(F("Humidity: "));
            Serial.print(homeHumidity);
            Serial.println(F("%"));
        }
        String tape = String(round(homeHumidity), 0) + "%";
        tape = tape.length() == 4 ? F("H") + tape : F("H ") + tape;
        drawString(tape);
    }
}

// Function to handle temperature reading errors
void Dht22_manager::handleTemperatureError() {
    dht22Start();  // Restart the DHT sensor
    if (Serial) Serial.println(F("Error reading temperature!"));
}

// Function to handle humidity reading errors
void Dht22_manager::handleHumidityError() {
    dht22Start();  // Restart the DHT sensor
    if (Serial) Serial.println(F("Error reading humidity!"));
}
