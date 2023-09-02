#pragma once


#include <WiFiManager.h>  //https://github.com/tzapu/WiFiManager



void wifi_init() {

  Serial.println("Start load WIFI");
  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;

  //wifiManager.debugPlatformInfo();

  //reset saved settings
  //wifiManager.resetSettings();

  //set custom ip for portal
  //wifiManager.setAPStaticIPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0));

  //wifiManager.addParameter(&custom_blynk_token);
  //sets timeout until configuration portal gets turned off
  //useful to make it all retry or go to sleep
  //in seconds
  wifiManager.setConnectTimeout(180);

  // set dark theme
  wifiManager.setClass("invert");

  bool res;
  //fetches ssid and pass from eeprom and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration
  res = wifiManager.autoConnect("AutoConnectAP", "flagman88");
  //or use this for auto generated name ESP + ChipID
  //wifiManager.autoConnect();

  if (!res) {
    Serial.println("Failed to connect or hit timeout");
    ESP.restart();
  } else {
    //WiFi.mode(WIFI_STA);

    wifiManager.stopWebPortal();

    WiFi.setAutoReconnect(true);
    WiFi.persistent(true);

    //if you get here you have connected to the WiFi
    Serial.println("connected...yeey :)");
    Serial.println("Ready");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    Serial.println("WIFI handled");
  }
}



void wifi_reset() {
  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;

  //reset saved settings
  wifiManager.resetSettings();
}
