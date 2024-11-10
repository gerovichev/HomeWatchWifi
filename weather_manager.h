#ifndef WEATHER_MANAGER_H
#define WEATHER_MANAGER_H

#include <Arduino.h>

class WeatherManager {
public:
    // Constructor
    WeatherManager();

    // Methods to interact with weather data
    void readWeather();                       // Reads weather data (replace with real data fetch)
    void printWeatherToScreen() const;        // Prints all weather data
    void printMaxTempToScreen() const;        // Prints max temperature
    void printPressureToScreen() const;       // Prints pressure
    void printHumidityToScreen() const;       // Prints humidity
    void printDescriptionWeatherToScreen() const; // Prints weather description

private:
    // Member variables to store weather data
    unsigned int temperature;
    unsigned int temp_max;
    unsigned int pressure;
    unsigned int humidity;
    unsigned int main_ext_humidity;
    String description_weather;
};

#endif // WEATHER_MANAGER_H
