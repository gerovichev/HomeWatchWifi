#include "dht22_manager.h"

// Define the DHT sensor object
DHT_Unified dht(DHTPIN, DHTTYPE);

// Define global variables
float homeTemp = 0.0;
float homeHumidity = 0.0;

// Function to initialize the DHT22 sensor and set home temperature
void dht22Start() {
    if (IS_DHT_CONNECTED) {
        dht.begin();
        sensor_t sensor;

        // Print temperature sensor details
        dht.temperature().getSensor(&sensor);
        printSensorDetails(sensor, "Temperature");

        // Print humidity sensor details
        dht.humidity().getSensor(&sensor);
        printSensorDetails(sensor, "Humidity");

        setHomeTemp();  // Read and set initial home temperature
    }
}

// Function to read and set the home temperature
void setHomeTemp() {
    sensors_event_t event;
    dht.temperature().getEvent(&event);
    if (isnan(event.temperature)) {
        handleTemperatureError();
    } else {
        homeTemp = event.temperature;
    }
}

// Function to print home temperature to the display
void printHomeTemp() {
    readAndPrintTemperature();
}

// Function to print humidity to the display
void printHumidity() {
    readAndPrintHumidity();
}

// Function to print detailed sensor information
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
void readAndPrintTemperature() {
    sensors_event_t event;
    dht.temperature().getEvent(&event);
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
void readAndPrintHumidity() {
    sensors_event_t event;
    dht.humidity().getEvent(&event);
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
void handleTemperatureError() {
    dht22Start();  // Restart the DHT sensor
    if (Serial) Serial.println(F("Error reading temperature!"));
}

// Function to handle humidity reading errors
void handleHumidityError() {
    dht22Start();  // Restart the DHT sensor
    if (Serial) Serial.println(F("Error reading humidity!"));
}
