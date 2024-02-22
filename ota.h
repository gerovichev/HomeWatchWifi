#pragma once

#include <ArduinoOTA.h>

//#include "clock_process.h"

void ota_init() {

  ArduinoOTA.setHostname((hostname_m + version_prg).c_str());
  // Hostname defaults to esp8266-[ChipID]
  //ArduinoOTA.setHostname(hostname_m);
  Serial.printf("New hostname: %s\n", ArduinoOTA.getHostname().c_str());

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else  // U_SPIFFS
      type = "filesystem";

    detachInterrupt_clock_process();

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    printText("Update");
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    printText("Restart");
    Serial.println("\nEnd");
    ESP.restart();
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    printText(String((progress / (total / 100)), DEC) + " %");
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
}
