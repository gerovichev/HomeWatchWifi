#pragma once

#include <WiFiClientSecureBearSSL.h>
#include <ArduinoJson.h>
#include "global.h"

unsigned int temperature;
unsigned int temp_max;
unsigned int pressure;
unsigned int main_ext_humidity;
String description_weather;

void readWeather() {
  if (Serial) Serial.println(F("Start get weather"));

  BearSSL::WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;
  http.setTimeout(1500); // Set the timeout

  String path = F("https://api.openweathermap.org/data/3.0/onecall?lat=") + String(latitude, 2) + F("&lon=") + String(longitude, 2) 
                + F("&units=metric&exclude=minutely,hourly,daily,alerts&appid=") + appidWeather + F("&lang=") + lang_weather;

  if (Serial) Serial.println(path);

  int attempts = 0;
  const int maxAttempts = 3;
  bool success = false;

  while (attempts < maxAttempts && !success) {
    if (http.begin(client, path)) {
      if (Serial) Serial.println(F("Start weather attempt ") + String(attempts + 1));
      int httpCode = http.GET();  // Send the request

      if (httpCode == HTTP_CODE_OK) {  // Check the returning code
        String payload = http.getString();  // Get the request response payload
        if (Serial) Serial.println("payload: " + payload);

        JsonDocument doc; 
        DeserializationError error = deserializeJson(doc, payload);
        // Test if parsing succeeds
        if (!error) {
          JsonObject current = doc["current"];
          unsigned int timezone_offset = doc["timezone_offset"]; 
          sunrise = current["sunrise"];
          if (Serial) Serial.println("sunrise1: " + formatTime(sunrise));
          sunset = current["sunset"];
          sunrise = sunrise + timezone_offset;
          if (Serial) Serial.println("sunrise2: " + formatTime(sunrise));
          sunset = sunset + timezone_offset;

          float current_temp = current["temp"];
          temperature = (int)floor(current_temp + 0.5);
          float current_feels_like = current["feels_like"];
          temp_max = (int)floor(current_feels_like + 0.5);
          int current_pressure = current["pressure"];
          pressure = current_pressure * 0.75006375541921;
          main_ext_humidity = current["humidity"];

          JsonObject current_weather_0 = current["weather"][0];
          const char* current_weather_0_description = current_weather_0["description"];
          description_weather = String(current_weather_0_description);

          success = true;  // Set success flag
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

void printWeatherToScreen() {
    String tape = String(temperature, DEC) + getGradValue() + F("C");
    drawString(tape);
}

void printMaxTempToScreen() {
    String tape = F("Feels like ") + String(temp_max, DEC) + getGradValue() + F("C");
    drawString(tape);
}

void printPressureToScreen() {
    String tape = String(pressure, DEC) + F("mm");
    drawString(tape);
}

void printHumidityToScreen() {
    String tape = String(main_ext_humidity, DEC) + F("%");
    drawString(tape);
}

void printDescriptionWeatherToScreen() {
  description_weather.toUpperCase();
  drawString(description_weather);
}
