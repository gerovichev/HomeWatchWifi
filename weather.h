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
    Serial.println("Start get weather");

    BearSSL::WiFiClientSecure client;
    client.setInsecure();

    HTTPClient http;
    String path = "https://api.openweathermap.org/data/3.0/onecall?lat=" + String(latitude, 2) + "&lon=" + String(longitude, 2) 
                  + "&units=metric&exclude=minutely,hourly,daily,alerts&appid=" + appidWeather + "&lang=" + lang_weather;

    Serial.println(path);

    if (http.begin(client, path)) {
        Serial.println("Start weather");
        int httpCode = http.GET();

        if (httpCode == HTTP_CODE_OK) {
            String payload = http.getString();
            Serial.println("payload: ");
            Serial.println(payload);

            JsonDocument doc; 
            DeserializationError error = deserializeJson(doc, payload);

            if (!error) {
                JsonObject current = doc["current"];
                unsigned int timezone_offset = doc["timezone_offset"]; 
                sunrise = current["sunrise"];
                Serial.println("sunrise1: " + formatTime(sunrise));
                sunset = current["sunset"];
                sunrise += timezone_offset;
                Serial.println("sunrise2: " + formatTime(sunrise));
                sunset += timezone_offset;

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
            } else {
                Serial.println("JSON parsing error: " + String(error.c_str()));
            }
        } else {
            Serial.println("HTTP request failed, error: " + String(httpCode));
        }
        http.end();
    } else {
        Serial.println("HTTP begin failed");
    }
}

void printWeatherToScreen() {
    String tape = String(temperature, DEC) + getGradValue() + "C";
    int mv = 5 - tape.length();
    drawString(tape, mv);
}

void printMaxTempToScreen() {
    String tape = "Feels like " + String(temp_max, DEC) + getGradValue() + "C";
    drawString(tape, 0);
}

void printPressureToScreen() {
    String tape = String(pressure, DEC) + "mm";
    drawString(tape, 0);
}

void printHumidityToScreen() {
    String tape = String(main_ext_humidity, DEC) + "%";
    int mv = 5 - tape.length();
    drawString(tape, mv);
}

void printDescriptionWeatherToScreen() {
    description_weather.toUpperCase();
    int mv = (description_weather.length() < 10) ? 5 - description_weather.length() / 2 : 0;
    drawString(description_weather, mv);
}

void weather_init() {
    readWeather();
    //isReadWeather = false;
}
