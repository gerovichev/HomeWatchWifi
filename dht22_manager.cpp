#include "dht22_manager.h"
#include "logger.h"

// Removed duplicate globals — homeTemp and homeHumidity are class members,
// not file-scope globals. Member initialisation is done in the constructor below.

Dht22_manager::Dht22_manager()
    : DHT_Unified(DHTPIN, DHTTYPE), homeTemp(0.0f), homeHumidity(0.0f) {}

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
    LOG_DEBUG_FMT("%s", type);

    LOG_VERBOSE_FMT("Sensor: %s",      sensor.name);
    LOG_VERBOSE_FMT("Driver Ver: %d",  sensor.version);
    LOG_VERBOSE_FMT("Unique ID: %d",   sensor.sensor_id);

    const char* unit = (strcmp(type, "Temperature") == 0) ? " *C" : " %";

    LOG_VERBOSE_FMT("Max Value: %.2f%s",  sensor.max_value,  unit);
    LOG_VERBOSE_FMT("Min Value: %.2f%s",  sensor.min_value,  unit);
    LOG_VERBOSE_FMT("Resolution: %.4f%s", sensor.resolution, unit);
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
        LOG_VERBOSE_FMT("Temperature: %.2f *C", homeTemp);
        char tape[12];
        snprintf(tape, sizeof(tape), "T%d%cC",
                 (int)round(homeTemp), getGradValue());
        drawString(String(tape));
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
        LOG_VERBOSE_FMT("Humidity: %.2f%%", homeHumidity);
        int h = (int)round(homeHumidity);
        char tape[8];
        // Pad with space when value is 2 digits to keep display aligned
        snprintf(tape, sizeof(tape), h >= 100 ? "H%d%%" : "H %d%%", h);
        drawString(String(tape));
    }
}

// Function to handle temperature reading errors
// Bug fix #10: previously called dht22Start() which calls setHomeTemp() which
// calls handleTemperatureError() again — infinite recursion on a disconnected
// sensor.  On ESP8266's ~4 KB stack this overflows very quickly.
// Now just logs and returns; the sensor will be re-read on the next cycle.
void Dht22_manager::handleTemperatureError() {
    LOG_ERROR_F("Error reading temperature!");
}

// Function to handle humidity reading errors
void Dht22_manager::handleHumidityError() {
    LOG_ERROR_F("Error reading humidity!");
}
