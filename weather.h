#pragma once

#include <WiFiClientSecureBearSSL.h>
#include "global.h"

unsigned int temperature;

unsigned int temp_max;

unsigned int pressure;

unsigned int main_ext_humidity;



String description_weather;



void readWeather() {
  Serial.println("Start get weather");


  //std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
  BearSSL::WiFiClientSecure client;
  client.setInsecure();

  //bool mfln = client.probeMaxFragmentLength("api.openweathermap.org", 443, 512);
  //if (mfln) { client.setBufferSizes(512, 512); }

  HTTPClient http;  //Declare an object of class HTTPClient

  String path = "https://api.openweathermap.org/data/3.0/onecall?lat=" + String(latitude, 2) + "&lon=" + String(longitude, 2) 
                + "&units=metric&exclude=minutely,hourly,daily,alerts&appid=" + appidWeather + "&lang=" + lang_weather;  //ru


  Serial.println(path);

  http.begin(client, path);

  //http.begin(path);

  //http.begin(path, "EE AA 58 6D 4F 1F 42 F4 18 5B 7F B0 F2 0A 4C DD 97 47 7D 99");
  int httpCode = http.GET();  //Send the request

  if (httpCode == HTTP_CODE_OK) {  //Check the returning code

    String payload = http.getString();  //Get the request response payload

    Serial.println("payload: ");
    Serial.println(payload);

    DynamicJsonDocument doc(payload.length() * 2);
    DeserializationError error = deserializeJson(doc, payload);
    // Test if parsing succeeds.
    if (!error) {
//float lat = doc["lat"]; // 31.65
//float lon = doc["lon"]; // 34.56
//const char* timezone = doc["timezone"]; // "Asia/Jerusalem"
//int timezone_offset = doc["timezone_offset"]; // 10800

JsonObject current = doc["current"];
//long current_dt = current["dt"]; // 1713606424
      sunrise = current["sunrise"]; // 1713582434
      sunset = current["sunset"]; // 1713629616

      float current_temp = current["temp"]; // 24.32
      temperature = (int)floor(current_temp + 0.5);
      float current_feels_like = current["feels_like"]; // 24.17
      temp_max = (int)floor(current_feels_like + 0.5);
      int current_pressure = current["pressure"]; // 1012
      pressure = current_pressure * 0.75006375541921;
      main_ext_humidity = current["humidity"]; // 52
/*float current_dew_point = current["dew_point"]; // 13.84
float current_uvi = current["uvi"]; // 7.92
int current_clouds = current["clouds"]; // 88
int current_visibility = current["visibility"]; // 10000
float current_wind_speed = current["wind_speed"]; // 3.85
int current_wind_deg = current["wind_deg"]; // 322
float current_wind_gust = current["wind_gust"]; // 2.94
*/

      JsonObject current_weather_0 = current["weather"][0];
      const char* current_weather_0_description = current_weather_0["description"]; // "overcast clouds"
      description_weather = String(current_weather_0_description);





/*
      JsonObject root = doc.as<JsonObject>();

      //DynamicJsonBuffer jsonBuffer(500);

      //    JsonObject& root = jsonBuffer.parseObject(payload);

      double main_temp = root["main"]["temp"];
      temperature = (int)floor(main_temp + 0.5);

      double main_temp_max = root["main"]["temp_max"];
      temp_max = (int)floor(main_temp_max + 0.5);

      double main_pressure = root["main"]["pressure"];
      pressure = main_pressure * 0.75006375541921;

      main_ext_humidity = root["main"]["humidity"];



      const char* description_weather_ch = root["weather"][0]["description"];
      description_weather = String(description_weather_ch);

      sunrise = root["sys"]["sunrise"];
      sunset = root["sys"]["sunset"];*/


    } else {
      drawString("No weather", 0);
      //delay(500);
    }

  } else {
    drawString("No weather 1 - " + String(httpCode, DEC), 0);
    //delay(500);
  }

  http.end();  //Close connection
}

void printWeatherToScreen() {

  /*for (int i=0; i<256; i++){
      String tape11 = String(i, DEC) + " " + ((char)i) ; //247
      drawString(tape11, 0);
      delay(1000);
    }*/

  String tape = String(temperature, DEC) + getGradValue() + "C";  //247
  int mv = 5 - tape.length();
  drawString(tape, mv);
}

void printMaxTempToScreen() {

  /*for (int i=0; i<256; i++){
      String tape11 = String(i, DEC) + " " + ((char)i) ; //247
      drawString(tape11, 0);
      delay(1000);
    }*/

  String tape = "Feels like " + String(temp_max, DEC) + getGradValue() + "C";  //247
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
  int mv = 0;
  description_weather.toUpperCase();
  if (description_weather.length() < 10) {
    mv = 5 - description_weather.length() / 2;
  }
  drawString(description_weather, mv);
}

void weather_init() {
  readWeather();

  //isReadWeather = false;
}
