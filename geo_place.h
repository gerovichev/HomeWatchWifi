#pragma once

#include <WifiLocation.h>
//#include <bingMapsGeocoding.h>
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
  Serial.println("LittleFS opened: " + result);

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
      Serial.println(F("Failed to read file, using default configuration"));

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
  Serial.println("LittleFS opened: ==");
  Serial.println(result);

  Serial.println("LittleFS file removed");

  Serial.println(filenamecnf);

  // Open file for writing
  File file = LittleFS.open(filenamecnf, "w");

  Serial.println("LittleFS file opened");

  if (!file) {
    Serial.println(F("Failed to create file"));
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
    Serial.println(F("Failed to write to file"));
  } else {
    Serial.println(F("Success to write to file"));
  }

  // Close the file
  file.close();
  LittleFS.end();
}

// Set time via NTP, as required for x.509 validation
void setClock() {
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");

  Serial.print("Waiting for NTP time sync: ");
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  Serial.print("\n");
  Serial.print("Current time: ");
  Serial.print(asctime(&timeinfo));
}

void getLocationAPI(String ip) {

  Serial.println("Start get location: really calling google API");

  setClock();

  WifiLocation location(googleApiKey);

  location_t loc = location.getGeoFromWiFi();
  //delay(1000);

  Serial.println("Result: " + location.wlStatusStr(location.getStatus()));
  if (!location.wlStatusStr(location.getStatus()).equals("OK")) {
    Serial.println("Returned status not OK");
    //ESP.restart();
    //return getLocationAPI();
  }
  Serial.println("Location request data");
  Serial.println(location.getSurroundingWiFiJson());
  latitude = loc.lat;
  longitude = loc.lon;

  Serial.println("Latitude: " + String(latitude, 7));
  Serial.println("Longitude: " + String(longitude, 7));
  Serial.println("Accuracy: " + String(loc.accuracy));

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
  Serial.println("Finish get location: really calling google API");
}

String getIp() {

  Serial.println("Start get IP");

  String payload;  // = "176.230.121.195";

  std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);


  //client->setFingerprint(fingerprint);
  client->setInsecure();


  HTTPClient http;  //Declare an object of class HTTPClient

  String path = "https://api.ipify.org";

  Serial.println(path);

  http.begin(*client, path);

  //delay(1000);

  int httpCode = http.GET();  //Send the request
  delay(500);

  Serial.println("get ip ran");

  if (httpCode == HTTP_CODE_OK) {  //Check the returning code

    payload = http.getString();  //Get the request response payload
  } else {
    Serial.print("Returned status not OK: ");
    Serial.println(httpCode);
    //ESP.restart();
  }

  http.end();  //Close connection


  Serial.print("Got ip: ");
  Serial.println(payload);
  return payload;
}

void location_init() {

  ip = getIp();

  loadConfiguration();

  if (ip.equals(config.ip) && config.latitude != 0) {
    Serial.println("Set location");
    latitude = config.latitude;
    longitude = config.longitude;
    //city_name = config.city_name;
    Serial.println("Latitude: " + String(latitude, 7));
    Serial.println("Longitude: " + String(longitude, 7));
  } else {
    getLocationAPI(ip);
    saveConfiguration();
  }
}
