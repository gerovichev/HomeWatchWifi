#include "location_manager.h"
#include <WifiLocation.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>

// Define global variables
String ip;
float latitude = 31.66;
float longitude = 34.56;
Config config;
int maxAttemptsLoc = 3;

// Path for configuration file
const char* filenamecnf = "/config.txt";

// Loads the configuration from a file
void loadConfiguration() {
  int result = LittleFS.begin();
  if (Serial) Serial.println(F("LittleFS opened: ") + String(result));

  File file = LittleFS.open(filenamecnf, "r");
  if (file) {
    DynamicJsonDocument doc(1024);  // Use appropriate capacity
    DeserializationError error = deserializeJson(doc, file);

    if (error) {
      if (Serial) Serial.println(F("Failed to read file, using default configuration"));
    } else {
      config.latitude = doc["latitude"];
      config.longitude = doc["longitude"];
      config.ip = String(doc["ip"]);
    }

    file.close();
  }

  LittleFS.end();
}

// Saves the configuration to a file
void saveConfiguration() {
  int result = LittleFS.begin();
  if (Serial) Serial.println(F("LittleFS opened: ") + String(result));

  File file = LittleFS.open(filenamecnf, "w");
  if (!file) {
    if (Serial) Serial.println(F("Failed to create file"));
    return;
  }

  DynamicJsonDocument doc(1024);  // Use appropriate capacity
  doc["latitude"] = config.latitude;
  doc["longitude"] = config.longitude;
  doc["ip"] = config.ip;

  if (serializeJson(doc, file) == 0) {
    if (Serial) Serial.println(F("Failed to write to file"));
  } else {
    if (Serial) Serial.println(F("Success to write to file"));
  }

  file.close();
  LittleFS.end();
}

// Sets time via NTP for x.509 validation
void setClock() {
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");

  if (Serial) Serial.print(F("Waiting for NTP time sync: "));
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    delay(500);
    if (Serial) Serial.print(F("."));
    now = time(nullptr);
  }

  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  if (Serial) Serial.print(F("\nCurrent time: ") + String(asctime(&timeinfo)));
}

// Get location via Google API using WiFi data
void getLocationAPI(String ip) {
  if (Serial) Serial.println(F("Start get location: calling Google API"));

  setClock();

  WifiLocation location(googleApiKey);
  location_t loc = location.getGeoFromWiFi();

  if (!location.wlStatusStr(location.getStatus()).equals("OK")) {
    if (Serial) Serial.println(F("Returned status not OK"));
    return;
  }

  latitude = loc.lat;
  longitude = loc.lon;

  if (Serial) {
    Serial.println(F("Location request data"));
    Serial.println(location.getSurroundingWiFiJson());
    Serial.println(F("Latitude: ") + String(latitude, 7));
    Serial.println(F("Longitude: ") + String(longitude, 7));
    Serial.println(F("Accuracy: ") + String(loc.accuracy));
  }

  config.latitude = latitude;
  config.longitude = longitude;
  config.ip = ip;
}

// Get external IP address using an API
String getIp() {
  if (Serial) Serial.println(F("Start get IP"));

  String payload;
  BearSSL::WiFiClientSecure client;
  client.setInsecure();  // Skip certificate validation
  HTTPClient http;

  String path = "https://api.ipify.org";
  int attempts = 0;
  bool success = false;

  while (attempts < maxAttemptsLoc && !success) {
    if (http.begin(client, path)) {
      if (Serial) Serial.println(F("Start IP retrieval attempt ") + String(attempts + 1));
      int httpCode = http.GET();  // Send the request

      if (httpCode == HTTP_CODE_OK) {
        payload = http.getString();  // Get the response payload
        success = true;
        maxAttemptsLoc = 1;
      } else {
        if (Serial) Serial.print(F("Returned status not OK: ") + String(httpCode));
      }

      http.end();
    } else {
      Serial.println(F("Failed to begin HTTP connection"));
    }

    if (!success) {
      attempts++;
      if (attempts < maxAttemptsLoc) {
        if (Serial) Serial.println(F("Retrying... (") + String(attempts) + F("/") + String(maxAttemptsLoc) + F(")"));
        delay(2000);
      } else {
        if (Serial) Serial.println(F("Failed to get IP after ") + String(maxAttemptsLoc) + F(" attempts. Restarting..."));
        ESP.restart();
      }
    }
  }

  if (Serial) Serial.print(F("Got IP: ") + payload);
  return payload;
}

// Initialize location by loading config or calling API
void location_init() {
  ip = getIp();
  loadConfiguration();

  if (ip.equals(config.ip) && config.latitude != 0) {
    latitude = config.latitude;
    longitude = config.longitude;
    if (Serial) {
      Serial.println(F("Set location from config"));
      Serial.println(F("Latitude: ") + String(latitude, 7));
      Serial.println(F("Longitude: ") + String(longitude, 7));
    }
  } else {
    getLocationAPI(ip);
    saveConfiguration();
  }
}
