#include "weather_manager.h"
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <ArduinoJson.h>
#include "Secret.h"
#include "location_manager.h"

#define MAX_ATTEMPS 3

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
  if (Serial) Serial.println(F("Start get weather"));

  int maxAttempts = MAX_ATTEMPS;

  BearSSL::WiFiClientSecure client;
  client.setInsecure();  // Skip certificate validation
  HTTPClient http;
  http.setTimeout(1500);  // Set the timeout

  String path = F("https://api.openweathermap.org/data/3.0/onecall?lat=") + String(latitude, 2) + F("&lon=") + String(longitude, 2) 
                + F("&units=metric&exclude=minutely,hourly,daily,alerts&appid=") + appidWeather + F("&lang=") + lang_weather;

  if (Serial) Serial.println(path);

  int attempts = 0;

  bool success = false;

  while (attempts < maxAttempts && !success) {
    if (http.begin(client, path)) {
      if (Serial) Serial.println(F("Start weather attempt ") + String(attempts + 1));
      int httpCode = http.GET();  // Send the request

      if (httpCode == HTTP_CODE_OK) {  // Check the returning code
        String payload = http.getString();  // Get the request response payload
        if (Serial) Serial.println(F("payload: ") + payload);

        DynamicJsonDocument doc(2048);  // Allocate JsonDocument
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

          success = true;  // Set success flag
          maxAttempts = 1;
        } else {
          if (Serial) Serial.println(F("JSON deserialization failed: ") + String(error.c_str()));
        }
      } else {
        if (Serial) Serial.println(F("No weather response: ") + String(httpCode, DEC));
      }

      http.end();  // Close connection
    } else {
      if (Serial) Serial.println(F("Failed to begin HTTP connection"));
    }

    if (!success) {
      attempts++;
      if (attempts < maxAttempts) {
        if (Serial) Serial.println(F("Retrying... (") + String(attempts) + F("/") + String(maxAttempts) + F(")"));
        delay(2000);  // Wait for 2 seconds before retrying
      } else {
        if (Serial) Serial.println(F("Failed to get weather data after ") + String(maxAttempts) + F(" attempts."));
      }
    }
  }
}

// Function to print temperature on the screen
void WeatherManager::printWeatherToScreen() const{
  String tape = String(temperature, DEC) + getGradValue() + F("C");
  drawString(tape);
}

// Function to print feels-like temperature on the screen
void WeatherManager::printMaxTempToScreen() const{
  String tape = F("Feels like ") + String(temp_max, DEC) + getGradValue() + F("C");
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
