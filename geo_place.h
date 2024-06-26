#pragma once

#include <WifiLocation.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include <ESP8266HTTPClient.h>
#include <time.h>

#include "global.h"

String ip;
float latitude = 31.66;
float longitude = 34.56;

struct Config {
    float latitude;
    float longitude;
    String ip;
};

const char* filenamecnf = "/config.txt";
Config config;

// Loads the configuration from a file
void loadConfiguration() {
    if (!LittleFS.begin()) {
        Serial.println("Failed to mount file system");
        return;
    }

    File file = LittleFS.open(filenamecnf, "r");
    if (!file) {
        Serial.println("Failed to open config file");
        return;
    }

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, file);
    if (error) {
        Serial.println(F("Failed to read file, using default configuration"));
    } else {
        config.latitude = doc["latitude"];
        config.longitude = doc["longitude"];
        config.ip = String(doc["ip"]);
    }

    file.close();
    LittleFS.end();
}

// Saves the configuration to a file
void saveConfiguration() {
    if (!LittleFS.begin()) {
        Serial.println("Failed to mount file system");
        return;
    }

    LittleFS.remove(filenamecnf);
    File file = LittleFS.open(filenamecnf, "w");
    if (!file) {
        Serial.println(F("Failed to create file"));
        return;
    }

    JsonDocument doc;
    doc["latitude"] = config.latitude;
    doc["longitude"] = config.longitude;
    doc["ip"] = config.ip;

    if (serializeJson(doc, file) == 0) {
        Serial.println(F("Failed to write to file"));
    } else {
        Serial.println(F("Successfully wrote to file"));
    }

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
    Serial.print("\nCurrent time: ");
    Serial.print(asctime(&timeinfo));
}

void getLocationAPI(String ip) {
    Serial.println("Start get location: really calling Google API");

    setClock();
    WifiLocation location(googleApiKey);
    location_t loc = location.getGeoFromWiFi();

    Serial.println("Result: " + location.wlStatusStr(location.getStatus()));
    if (location.getStatus() != WL_STATUS_OK) {
        Serial.println("Returned status not OK");
        return;
    }

    latitude = loc.lat;
    longitude = loc.lon;
    Serial.println("Latitude: " + String(latitude, 7));
    Serial.println("Longitude: " + String(longitude, 7));
    Serial.println("Accuracy: " + String(loc.accuracy));

    config.latitude = latitude;
    config.longitude = longitude;
    config.ip = ip;

    Serial.println("Finish get location: really calling Google API");
}

String getIp() {
    Serial.println("Start get IP");

    std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);
    client->setInsecure();

    HTTPClient http;
    String path = "https://api.ipify.org";

    http.begin(*client, path);
    int httpCode = http.GET();
    String payload;

    if (httpCode == HTTP_CODE_OK) {
        payload = http.getString();
    } else {
        Serial.print("Returned status not OK: ");
        Serial.println(httpCode);
        ESP.restart();
    }

    http.end();

    Serial.print("Got IP: ");
    Serial.println(payload);
    return payload;
}

void location_init() {
    ip = getIp();
    loadConfiguration();

    if (ip.equals(config.ip) && config.latitude != 0) {
        Serial.println("Set location from config");
        latitude = config.latitude;
        longitude = config.longitude;
        Serial.println("Latitude: " + String(latitude, 7));
        Serial.println("Longitude: " + String(longitude, 7));
    } else {
        getLocationAPI(ip);
        saveConfiguration();
    }
}
