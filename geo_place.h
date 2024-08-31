#pragma once

#include <WifiLocation.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include <ESP8266HTTPClient.h>

#include "global.h"


String ip;
float latitude = 31.66;
float longitude = 34.56;

struct Config {
  float latitude;
  float longitude;
  String ip;
  //  String city_name;
};

const char* filenamecnf = "/config.txt";  // <- SD library uses 8.3 filenames
Config config;                            // <- global configuration object

// Loads the configuration from a file
void loadConfiguration() {

  // always use this to "mount" the filesystem
  int result = LittleFS.begin();
  if (Serial) Serial.println("LittleFS opened: " + result);

  // Open file for reading
  File file = LittleFS.open(filenamecnf, "r");

  if (file) {
    // Allocate a temporary JsonDocument
    // Don't forget to change the capacity to match your requirements.
    // Use arduinojson.org/v6/assistant to compute the capacity.
    JsonDocument doc;

    // Deserialize the JSON document
    DeserializationError error = deserializeJson(doc, file);
    if (error)
      if (Serial) Serial.println(F("Failed to read file, using default configuration"));

    // Copy values from the JsonDocument to the Config
    config.latitude = doc["latitude"];
    config.longitude = doc["longitude"];
    config.ip = String(doc["ip"]);

    //const char* name_ct = doc["city_name"];
    //config.city_name = String(name_ct);
    /*
    strlcpy(config.ip,                  // <- destination
            doc["ip"],  // <- source
            sizeof(config.ip));         // <- destination's capacity
  */
  }
  // Close the file (Curiously, File's destructor doesn't close the file)
  file.close();

  LittleFS.end();
}

// Saves the configuration to a file
void saveConfiguration() {

  // always use this to "mount" the filesystem
  int result = LittleFS.begin();
  if (Serial) 
  {
    Serial.println(F("LittleFS opened: =="));
    Serial.println(result);

    Serial.println(F("LittleFS file removed"));

    Serial.println(filenamecnf);
  }

  // Open file for writing
  File file = LittleFS.open(filenamecnf, "w");

  if (Serial) Serial.println(F("LittleFS file opened"));

  if (!file) {
    if (Serial) Serial.println(F("Failed to create file"));
    return;
  }

  // Allocate a temporary JsonDocument
  // Don't forget to change the capacity to match your requirements.
  // Use arduinojson.org/assistant to compute the capacity.
  JsonDocument doc;

  // Set the values in the document
  doc["latitude"] = config.latitude;
  doc["longitude"] = config.longitude;
  doc["ip"] = config.ip;
  //doc["city_name"] = config.city_name;

  // Serialize JSON to file
  if (serializeJson(doc, file) == 0) {
    if (Serial) Serial.println(F("Failed to write to file"));
  } else {
    if (Serial) Serial.println(F("Success to write to file"));
  }

  // Close the file
  file.close();
  LittleFS.end();
}

// Set time via NTP, as required for x.509 validation
void setClock() {
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");

  if (Serial) Serial.print(F("Waiting for NTP time sync: "));
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    delay(500);
    if (Serial) Serial.print(".");
    now = time(nullptr);
  }
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  if (Serial) Serial.print(F("\nCurrent time: ") + String(asctime(&timeinfo)));
}

void getLocationAPI(String ip) {

  if (Serial) Serial.println(F("Start get location: really calling google API"));

  setClock();

  WifiLocation location(googleApiKey);

  location_t loc = location.getGeoFromWiFi();
  //delay(1000);

  Serial.println(F("Result: ") + location.wlStatusStr(location.getStatus()));
  if (!location.wlStatusStr(location.getStatus()).equals("OK")) {
    if (Serial) Serial.println(F("Returned status not OK"));
    //ESP.restart();
    //return getLocationAPI();
  }
  latitude = loc.lat;
  longitude = loc.lon; 
  
  if (Serial) 
  {
    Serial.println(F("Location request data"));
    Serial.println(location.getSurroundingWiFiJson());


    Serial.println(F("Latitude: ") + String(latitude, 7));
    Serial.println(F("Longitude: ") + String(longitude, 7));
    Serial.println(F("Accuracy: ") + String(loc.accuracy));
  }
  config.latitude = latitude;
  config.longitude = longitude;

  config.ip = ip;

  //BingGeoCoding bingGeoCoding (bingMapsKey);
  /*
  //String ip = getIp();
  char ip_c[15];
  //ip.toCharArray(ip_c, 15);
  strlcpy(config.ip,           // <- destination
          ip,                // <- source
          sizeof(config.ip));  // <- destination's capacity

*/
  if (Serial) Serial.println(F("Finish get location: really calling google API"));
}

String getIp() {
  if (Serial) Serial.println(F("Start get IP"));

  String payload;

  BearSSL::WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;

  String path = "https://api.ipify.org";

  if (Serial) Serial.println(path);

  int attempts = 0;
  const int maxAttempts = 3;
  bool success = false;

  while (attempts < maxAttempts && !success) {
    if (http.begin(client, path)) {
      if (Serial) Serial.println(F("Start IP retrieval attempt ") + String(attempts + 1));
      int httpCode = http.GET();  // Send the request

      if (Serial) Serial.println(F("get ip ran"));

      if (httpCode == HTTP_CODE_OK) {  // Check the returning code
        payload = http.getString();  // Get the request response payload
        success = true;  // Set success flag
      } else {
        if (Serial) Serial.print(F("Returned status not OK: ") + String(httpCode));
      }

      http.end();  // Close connection
    } else {
      Serial.println(F("Failed to begin HTTP connection"));
    }

    if (!success) {
      attempts++;
      if (attempts < maxAttempts) {
        if (Serial) Serial.println(F("Retrying... (") + String(attempts) + F("/") + String(maxAttempts) + F(")"));
        delay(2000);  // Wait for 2 seconds before retrying
      } else {
        if (Serial) Serial.println(F("Failed to get IP after ") + String(maxAttempts) + F(" attempts. Restarting..."));
        ESP.restart();
      }
    }
  }

  if (Serial) Serial.print(F("Got IP: ") + payload);

  return payload;
}


void location_init() {

  ip = getIp();

  loadConfiguration();

    if (ip.equals(config.ip) && config.latitude != 0) {

        latitude = config.latitude;
        longitude = config.longitude;
        if (Serial) 
        {
          Serial.println(F("Set location from config")); 
          Serial.println(F("Latitude: ") + String(latitude, 7));
          Serial.println(F("Longitude: ") + String(longitude, 7));
        }
    } else {
        getLocationAPI(ip);
        saveConfiguration();
    }
}
