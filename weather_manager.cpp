#include "weather_manager.h"
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <ArduinoJson.h>
#include "Secret.h"
#include "location_manager.h"
#include "secure_client.h"
#include "constants.h"
#include "logger.h"

WeatherManager::WeatherManager()
{
    // Initialize weather data members here, if necessary
    temperature = 0;
    temp_max = 0;
    pressure = 0;
    humidity = 0;
    description_weather = "";
}

// Function to read weather data from the OpenWeather API
void WeatherManager::readWeather() {
  LOG_INFO_F("Fetching weather data from OpenWeatherMap...");

  int maxAttempts = Retry::MAX_ATTEMPTS_WEATHER;

  BearSSL::WiFiClientSecure client;
  setupSecureClient(client, "openweathermap.org");
  HTTPClient http;
  http.setTimeout(Timing::HTTP_TIMEOUT_MS);

  // Optimize URL construction to avoid multiple String concatenations
  char path[Buffer::PATH_BUFFER_SIZE];
  snprintf(path, sizeof(path), 
           "https://api.openweathermap.org/data/3.0/onecall?lat=%.2f&lon=%.2f&units=metric&exclude=minutely,hourly,daily,alerts&appid=%s&lang=%s",
           latitude, longitude, appidWeather, lang_weather.c_str());

  LOG_DEBUG("Weather API URL: " + String(path));

  int attempts = 0;
  bool success = false;

  while (attempts < maxAttempts && !success) {
    if (http.begin(client, path)) {
      LOG_DEBUG("Weather API attempt " + String(attempts + 1) + "/" + String(maxAttempts));
      int httpCode = http.GET();  // Send the request

      if (httpCode == HTTP_CODE_OK) {  // Check the returning code
        String payload = http.getString();  // Get the request response payload
        LOG_VERBOSE("Weather API response: " + payload);

        StaticJsonDocument<Buffer::JSON_WEATHER_SIZE> doc;  // Weather API response with current weather data
        DeserializationError error = deserializeJson(doc, payload);

        // Test if parsing succeeds
        if (!error) {
          JsonObject current = doc[F("current")];
          unsigned int timezone_offset = doc[F("timezone_offset")];
          sunrise = current[F("sunrise")];
          sunset = current[F("sunset")];
          sunrise += timezone_offset;
          sunset += timezone_offset;

          temperature = (int)floor((double)current[F("temp")] + 0.5);
          temp_max = (int)floor((double)current[F("feels_like")] + 0.5);
          pressure = (int)((double)current[F("pressure")] * 0.75006375541921);  // Convert pressure to mmHg
          main_ext_humidity = current[F("humidity")];

          JsonObject weather = current[F("weather")][0];
          description_weather = String(weather[F("description")]);
          description_weather.toUpperCase();

          LOG_INFO("Weather updated: " + String(temperature) + "°C, " + 
                   String(main_ext_humidity) + "%, " + String(pressure) + "mm");
          LOG_DEBUG("Feels like: " + String(temp_max) + "°C");
          LOG_DEBUG("Description: " + description_weather);

          success = true;  // Set success flag
          maxAttempts = 1;
        } else {
          LOG_ERROR("Weather JSON deserialization failed: " + String(error.c_str()));
        }
      } else {
        LOG_WARNING("Weather API HTTP error: " + String(httpCode));
      }

    } else {
      LOG_ERROR_F("Failed to begin weather HTTP connection");
    }

    if (!success) {
      attempts++;
      if (attempts < maxAttempts) {
        LOG_WARNING("Retrying weather request (" + String(attempts) + "/" + String(maxAttempts) + ")...");
        delay(Timing::RETRY_DELAY_MS);  // Wait before retrying
      } else {
        LOG_ERROR("Failed to get weather data after " + String(maxAttempts) + " attempts");
      }
    }
  }
  http.end();  // Close connection
}

// Function to print temperature on the screen
void WeatherManager::printWeatherToScreen() const{
  String tape = String(temperature, DEC) + getGradValue() + F("C");
  drawString(tape);
}

// Function to print feels-like temperature on the screen
void WeatherManager::printMaxTempToScreen() const{
  // Very short format to fit on display: "~25°C" (tilde ~ means "feels like")
  String tape = F("~") + String(temp_max, DEC) + getGradValue() + F("C");
  drawString(tape);
}

// Function to print pressure on the screen
void WeatherManager::printPressureToScreen() const{
  String tape = String(pressure, DEC) + F("mm");
  drawString(tape);
}

// Function to print humidity on the screen
void WeatherManager::printHumidityToScreen() const{
  String tape = String(main_ext_humidity, DEC) + F("%");
  drawString(tape);
}

// Function to print weather description on the screen
void WeatherManager::printDescriptionWeatherToScreen() const {  
  drawString(description_weather);
}
