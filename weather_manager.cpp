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
    main_ext_humidity = 0;
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

  LOG_DEBUG_FMT("Weather API URL: %s", path);

  int attempts = 0;
  bool success = false;

  while (attempts < maxAttempts && !success) {
    if (http.begin(client, path)) {
      LOG_DEBUG_FMT("Weather API attempt %d/%d", attempts + 1, maxAttempts);
      int httpCode = http.GET();  // Send the request

      if (httpCode == HTTP_CODE_OK) {  // Check the returning code
        String payload = http.getString();  // Get the request response payload
        LOG_VERBOSE_FMT("Weather API response: %s", payload.c_str());

        StaticJsonDocument<Buffer::JSON_WEATHER_SIZE> doc;  // Weather API response with current weather data
        DeserializationError error = deserializeJson(doc, payload);

        // Test if parsing succeeds
        if (!error) {
          JsonObject current = doc[F("current")];
          // Bug fix #6: timezone_offset can be negative (e.g. UTC-5 = -18000).
          // Store as int; sunrise/sunset are full epoch values so use unsigned long.
          int timezone_offset = (int)doc[F("timezone_offset")];
          sunrise = (unsigned long)((long)current[F("sunrise")] + timezone_offset);
          sunset  = (unsigned long)((long)current[F("sunset")]  + timezone_offset);

          temperature = (int)floor((double)current[F("temp")] + 0.5);
          temp_max = (int)floor((double)current[F("feels_like")] + 0.5);
          pressure = (int)((double)current[F("pressure")] * 0.75006375541921);  // Convert pressure to mmHg
          main_ext_humidity = current[F("humidity")];

          JsonObject weather = current[F("weather")][0];
          description_weather = String(weather[F("description")]);
          description_weather.toUpperCase();

          LOG_INFO_FMT("Weather updated: %d C, %u%%, %dmm", temperature,
                       main_ext_humidity, pressure);
          LOG_DEBUG_FMT("Feels like: %d C", temp_max);
          LOG_DEBUG_FMT("Description: %s", description_weather.c_str());

          success = true;  // Set success flag
          // Bug fix #7: removed redundant `maxAttempts = 1` — the while
          // condition already checks `!success`, so the loop exits correctly.
        } else {
          LOG_ERROR_FMT("Weather JSON deserialization failed: %s",
                        error.c_str());
        }
      } else {
        LOG_WARNING_FMT("Weather API HTTP error: %d", httpCode);
      }

    } else {
      LOG_ERROR_F("Failed to begin weather HTTP connection");
    }

    http.end();

    if (!success) {
      attempts++;
      if (attempts < maxAttempts) {
        LOG_WARNING_FMT("Retrying weather request (%d/%d)...", attempts, maxAttempts);
        delay(Timing::RETRY_DELAY_MS);
        yield(); // Allow system tasks
      } else {
        LOG_ERROR_FMT("Failed to get weather data after %d attempts", maxAttempts);
      }
    }
  }
}

// Function to print temperature on the screen
void WeatherManager::printWeatherToScreen() const{
  char tape[12];
  snprintf(tape, sizeof(tape), "%d%cC", temperature, getGradValue());
  drawString(String(tape));
}

// Function to print feels-like temperature on the screen
void WeatherManager::printMaxTempToScreen() const{
  // Very short format to fit on display: "~25°C" (tilde ~ means "feels like")
  char tape[12];
  snprintf(tape, sizeof(tape), "~%d%cC", temp_max, getGradValue());
  drawString(String(tape));
}

// Function to print pressure on the screen
void WeatherManager::printPressureToScreen() const{
  char tape[12];
  snprintf(tape, sizeof(tape), "%dmm", pressure);
  drawString(String(tape));
}

// Function to print humidity on the screen
void WeatherManager::printHumidityToScreen() const{
  char tape[8];
  snprintf(tape, sizeof(tape), "%u%%", main_ext_humidity);
  drawString(String(tape));
}

// Function to print weather description on the screen
void WeatherManager::printDescriptionWeatherToScreen() const {  
  drawString(description_weather);
}
