#include "TimeManager.h"
#include "secure_client.h"
#include "constants.h"
#include "logger.h"

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

time_t zoneStart;
time_t zoneEnd;
time_t timeNow;

int offset;
String city_name;
int maxAttemptsTimes = Retry::MAX_ATTEMPTS_TIMEZONE;


void getTimezone() {
  LOG_DEBUG_F("Checking timezone...");
  LOG_VERBOSE_FMT("Zone end: %lu, Current time: %lu", zoneEnd, timeNow);

  if (zoneEnd > timeNow) {
    time_t untilTimeMove = zoneEnd - timeNow;
    int daysUntilTimeMove = untilTimeMove / 86400;
    LOG_DEBUG_FMT("Days until timezone change: %d", daysUntilTimeMove);
    LOG_INFO_F("Timezone is current, no update needed");
    return;
  }

  LOG_INFO_F("Fetching timezone information...");
  
  BearSSL::WiFiClientSecure client;
  setupSecureClient(client, "timezonedb.com");
  HTTPClient http;

  // Optimize URL construction to avoid multiple String concatenations
  char path[Buffer::PATH_BUFFER_SIZE];
  snprintf(path, sizeof(path),
           "https://api.timezonedb.com/v2.1/get-time-zone?key=%s&format=json&lat=%.2f&lng=%.2f&by=position",
           apiKeyTimezone, latitude, longitude);
  
  LOG_DEBUG("Timezone API URL: " + String(path));

  int attempts = 0;
  bool success = false;

  while (attempts < maxAttemptsTimes && !success) {
    if (http.begin(client, path)) {
      LOG_DEBUG_FMT("Timezone API attempt %d/%d", attempts + 1, maxAttemptsTimes);
      int httpCode = http.GET();  // Send the request

      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        LOG_VERBOSE("Timezone API response: " + payload);

        StaticJsonDocument<Buffer::JSON_TIMEZONE_SIZE> doc;  // Timezone API response: status, offset, zoneStart, zoneEnd, cityName
        DeserializationError error = deserializeJson(doc, payload);
        if (!error) {
          LOG_VERBOSE_F("Timezone JSON deserialization succeeded");
          JsonObject root = doc.as<JsonObject>();

          const char* status = root["status"];
          if (strcmp(status, "OK") == 0) {
            offset = (int)root["gmtOffset"];
            timeClient.setTimeOffset(offset);

            zoneStart = (unsigned long)root["zoneStart"];
            zoneEnd = (unsigned long)root["zoneEnd"];

            const char* name_ct = root["cityName"];
            city_name = String(name_ct);

            LOG_INFO_FMT("Timezone updated: %s (UTC%s%d)", city_name.c_str(), offset >= 0 ? "+" : "", offset/3600);
            LOG_DEBUG_FMT("GMT offset: %ld seconds", offset);

            success = true;
            // Bug fix #7: removed redundant `maxAttemptsTimes = 1` — the
            // while condition already checks `!success`.
          }
        } else {
          LOG_ERROR("Timezone JSON deserialization failed: " + String(error.c_str()));
        }
      } else {
        LOG_WARNING_FMT("Timezone API HTTP error: %d", httpCode);
      }
    } else {
      LOG_ERROR_F("Failed to begin timezone HTTP connection");
    }

    http.end();

    if (!success) {
      attempts++;
      if (attempts < maxAttemptsTimes) {
        LOG_WARNING_FMT("Retrying timezone request (%d/%d)...", attempts, maxAttemptsTimes);
        delay(Timing::RETRY_DELAY_MS);
      } else {
        LOG_ERROR_FMT("Failed to get timezone data after %d attempts", maxAttemptsTimes);    
      }
    }
  }
}

void printTimeToScreen() {
  String tape = timeClient.getFormattedTime().substring(0, 5);
  drawString(tape);
}

void printDateToScreen() {
  time_t epochTime = timeClient.getEpochTime();
  // Bug fix #13: removed redundant cast — epochTime is already time_t.
  struct tm* ptm = gmtime(&epochTime);
  String tape = getNumberWithZerro(ptm->tm_mday) + F("/") + getNumberWithZerro(ptm->tm_mon + 1);
  drawString(tape);
}

void printDayToScreen() {
  String tape = String(getDayOfWeek(timeClient.getDay()));
  drawString(tape);
}

void printCityToScreen() {
  displayTextInSetup(city_name);
}

void ntp_init() {
  LOG_INFO_F("Initializing NTP client...");
  timeClient.begin();
  getTimezone();
  timeClient.update();
  LOG_INFO("NTP synchronized, current time: " + timeClient.getFormattedTime());
  printCityToScreen();
}
