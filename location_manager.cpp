#include "location_manager.h"
#include <WifiLocation.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include "secure_client.h"
#include "constants.h"
#include "logger.h"

// Define global variables
String ip;
float latitude = 31.66;
float longitude = 34.56;
Config config;
int maxAttemptsLoc = Retry::MAX_ATTEMPTS_LOCATION;

// Path for configuration file
const char* filenamecnf = "/config.txt";

// Loads the configuration from a file
void loadConfiguration() {
  int result = LittleFS.begin();
  LOG_DEBUG("LittleFS opened with result: " + String(result));

  File file = LittleFS.open(filenamecnf, "r");
  if (file) {
    StaticJsonDocument<Buffer::JSON_LOCATION_SIZE> doc;  // Location config is small: lat, lon, ip
    DeserializationError error = deserializeJson(doc, file);

    if (error) {
      LOG_WARNING_F("Failed to read location config file, using defaults");
    } else {
      config.latitude = doc["latitude"];
      config.longitude = doc["longitude"];
      config.ip = String(doc["ip"]);
      LOG_INFO("Loaded location config: lat=" + String(config.latitude, 6) + 
               ", lon=" + String(config.longitude, 6) + ", ip=" + config.ip);
    }

    file.close();
  } else {
    LOG_WARNING_F("Location config file not found");
  }

  LittleFS.end();
}

// Saves the configuration to a file
void saveConfiguration() {
  int result = LittleFS.begin();
  LOG_DEBUG("LittleFS opened for writing: " + String(result));

  File file = LittleFS.open(filenamecnf, "w");
  if (!file) {
    LOG_ERROR_F("Failed to create location config file");
    return;
  }

    StaticJsonDocument<Buffer::JSON_LOCATION_SIZE> doc;  // Location config is small: lat, lon, ip
    doc["latitude"] = config.latitude;
  doc["longitude"] = config.longitude;
  doc["ip"] = config.ip;

  if (serializeJson(doc, file) == 0) {
    LOG_ERROR_F("Failed to write location config to file");
  } else {
    LOG_INFO("Location config saved: lat=" + String(config.latitude, 6) + 
             ", lon=" + String(config.longitude, 6) + ", ip=" + config.ip);
  }

  file.close();
  LittleFS.end();
}

// Sets time via NTP for x.509 validation
void setClock() {
  LOG_DEBUG_F("Setting system clock for SSL validation...");
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");

  time_t now = time(nullptr);
  int waitCount = 0;
  while (now < 8 * 3600 * 2) {
    delay(Timing::NTP_SYNC_WAIT_MS);
    if (++waitCount % 10 == 0) {
      LOG_VERBOSE_F("Waiting for NTP time sync...");
    }
    now = time(nullptr);
  }

  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  LOG_DEBUG("System clock set: " + String(asctime(&timeinfo)));
}

// Get location via Google API using WiFi data
void getLocationAPI(String ip) {
  LOG_INFO_F("Fetching location via Google Geolocation API...");

  setClock();

  WifiLocation location(googleApiKey);
  location_t loc = location.getGeoFromWiFi();

  if (!location.wlStatusStr(location.getStatus()).equals("OK")) {
    LOG_ERROR("Google Geolocation API returned status: " + location.wlStatusStr(location.getStatus()));
    return;
  }

  latitude = loc.lat;
  longitude = loc.lon;

  LOG_INFO("Location updated: lat=" + String(latitude, 7) + ", lon=" + String(longitude, 7));
  LOG_DEBUG("Location accuracy: " + String(loc.accuracy) + " meters");
  LOG_VERBOSE("WiFi scan data: " + location.getSurroundingWiFiJson());

  config.latitude = latitude;
  config.longitude = longitude;
  config.ip = ip;
}

// Get external IP address using an API
String getIp() {
  LOG_INFO_F("Fetching external IP address...");

  String payload;
  BearSSL::WiFiClientSecure client;
  setupSecureClient(client, "ipify.org");
  HTTPClient http;

  String path = "https://api.ipify.org";
  int attempts = 0;
  bool success = false;

  while (attempts < maxAttemptsLoc && !success) {
    if (http.begin(client, path)) {
      LOG_DEBUG("IP retrieval attempt " + String(attempts + 1) + "/" + String(maxAttemptsLoc));
      int httpCode = http.GET();  // Send the request

      if (httpCode == HTTP_CODE_OK) {
        payload = http.getString();  // Get the response payload
        LOG_INFO("External IP retrieved: " + payload);
        success = true;
        maxAttemptsLoc = 1;
      } else {
        LOG_WARNING("IP retrieval HTTP error: " + String(httpCode));
      }

      http.end();
    } else {
      LOG_ERROR_F("Failed to begin IP retrieval HTTP connection");
    }

    if (!success) {
      attempts++;
      if (attempts < maxAttemptsLoc) {
        LOG_WARNING("Retrying IP retrieval (" + String(attempts) + "/" + String(maxAttemptsLoc) + ")...");
        delay(Timing::RETRY_DELAY_MS);
      } else {
        LOG_ERROR("Failed to get IP after " + String(maxAttemptsLoc) + " attempts.");
        // Don't restart immediately - allow device to continue with cached location if available
        // Only restart if this is critical for device operation
        if (config.latitude == 0 && config.longitude == 0) {
          LOG_ERROR_F("No cached location available, restarting device...");
          delay(1000);
          ESP.restart();
        } else {
          LOG_WARNING_F("Using cached location due to IP retrieval failure");
        }
      }
    }
  }

  return payload;
}

// Initialize location by loading config or calling API
void location_init() {
  LOG_INFO_F("Initializing location services...");
  
  ip = getIp();
  loadConfiguration();

  if (ip.equals(config.ip) && config.latitude != 0) {
    latitude = config.latitude;
    longitude = config.longitude;
    LOG_INFO("Using cached location: lat=" + String(latitude, 7) + 
             ", lon=" + String(longitude, 7));
  } else {
    LOG_INFO_F("IP changed or no cached location, fetching new location...");
    getLocationAPI(ip);
    saveConfiguration();
  }
}
