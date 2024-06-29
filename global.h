#pragma once

#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>

#include "secret.h"

String lang_weather;
unsigned int sunrise;
unsigned int sunset;

#include "led.h"

String version_prg = "300624";

char grad = '\x60';  //247;

char getGradValue() {

  /*if (lang_weather.compareTo("en") == 0) {
    grad = 176;  //;248
  }*/
  return grad;
}

float humidity_delta = 0.00;

String hostname_m;

boolean isOTAreq = true;
boolean isMQTT = false;
String nameofWatch;

String macAddrSt;

//static const char daysOfTheWeek[7][6] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
String daysOfTheWeek[7];  // = {"Вс.", "Пн.", "Вт.", "Ср.", "Чт.", "Пт.", "Сб."};

boolean IS_DHT_CONNECTED = false;

bool isWebClientNeeded = true;

boolean isReadWeather = true;

void initPerDevice() {

  setDeviceConfig();

  String macAddr = WiFi.macAddress();
  Serial.print("MAC: ");
  Serial.println(macAddr);

  macAddrSt = macAddr;

  if (configMap.find(macAddr) != configMap.end()) {
    DeviceConfig& config = configMap[macAddr];

    lang_weather = config.lang_weather;
    hostname_m = config.hostname_m;
    IS_DHT_CONNECTED = config.IS_DHT_CONNECTED;
    isWebClientNeeded = config.isWebClientNeeded;
    isReadWeather = config.isReadWeather;
    humidity_delta = config.humidity_delta;
    nameofWatch = config.nameofWatch;
    isOTAreq = config.isOTAreq;
    isMQTT = config.isMQTT;
    setIntensity(config.intensity);
    mqtt_topic_str = hostname_m + mqtt_topic;


  } else {
    lang_weather = "en";
    hostname_m = "ESP_Unknown";
    IS_DHT_CONNECTED = false;
    isWebClientNeeded = true;
    isReadWeather = true;
    nameofWatch = "New";
  }

  Serial.println("Host name: " + hostname_m);

  if (!lang_weather.compareTo("ru")) {
    String daysOfTheWeekT[7] = { "Вс.", "Пн.", "Вт.", "Ср.", "Чт.", "Пт.", "Сб." };

    for (int i = 0; i < 7; i++)
      daysOfTheWeek[i] = daysOfTheWeekT[i];

  } else {
    String daysOfTheWeekT[7] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };

    for (int i = 0; i < 7; i++)
      daysOfTheWeek[i] = daysOfTheWeekT[i];
  }
}

void verifyWifi() {
  while (WiFi.status() != WL_CONNECTED || WiFi.localIP() == IPAddress(0, 0, 0, 0))
    WiFi.reconnect();
}

String getNumberWithZerro(int dig) {
  String tape = String(dig, DEC);
  if (tape.length() == 1) {
    tape = "0" + tape;
  }
  return tape;
}

void drawString(String tape, int start) {
  drawStringMax(tape, start);
}

