#include "dht22_manager.h"
#include "logger.h"

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
    LOG_DEBUG_F("------------------------------------");
    LOG_DEBUG(String(type));

    LOG_VERBOSE("Sensor: " + String(sensor.name));
    LOG_VERBOSE("Driver Ver: " + String(sensor.version));
    LOG_VERBOSE("Unique ID: " + String(sensor.sensor_id));

    const char* unit = (strcmp(type, "Temperature") == 0) ? " *C" : " %";

    LOG_VERBOSE("Max Value: " + String(sensor.max_value) + String(unit));
    LOG_VERBOSE("Min Value: " + String(sensor.min_value) + String(unit));
    LOG_VERBOSE("Resolution: " + String(sensor.resolution) + String(unit));
    LOG_DEBUG_F("------------------------------------");
}

// Function to read and print temperature to the display
void Dht22_manager::readAndPrintTemperature() {
    sensors_event_t event;
    temperature().getEvent(&event);
    if (isnan(event.temperature)) {
        handleTemperatureError();
    } else {
        homeTemp = event.temperature;
        LOG_VERBOSE("Temperature: " + String(homeTemp, 2) + " *C");
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
        LOG_VERBOSE("Humidity: " + String(homeHumidity, 2) + "%");
        String tape = String(round(homeHumidity), 0) + "%";
        tape = tape.length() == 4 ? F("H") + tape : F("H ") + tape;
        drawString(tape);
    }
}

// Function to handle temperature reading errors
void Dht22_manager::handleTemperatureError() {
    dht22Start();  // Restart the DHT sensor
    LOG_ERROR_F("Error reading temperature!");
}

// Function to handle humidity reading errors
void Dht22_manager::handleHumidityError() {
    dht22Start();  // Restart the DHT sensor
    LOG_ERROR_F("Error reading humidity!");
}
