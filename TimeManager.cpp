#include "TimeManager.h"
#include "constants.h"
#include "logger.h"
#include "root_certs.h"

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

  // BearSSL needs a real system clock to validate the certificate's
  // notBefore/notAfter dates; this is a no-op once NTP has already synced.
  setClock();

  // rootCA is declared before client so that client is destroyed first — the
  // client must never outlive the trust anchors it points at.
  BearSSL::X509List rootCA(ISRG_ROOT_X1);
  BearSSL::WiFiClientSecure client;
  client.setTrustAnchors(&rootCA);
  client.setBufferSizes(Buffer::TLS_RX_SIZE, Buffer::TLS_TX_SIZE);
  HTTPClient http;

  // Optimize URL construction to avoid multiple String concatenations
  char path[Buffer::PATH_BUFFER_SIZE];
  snprintf(path, sizeof(path),
           "https://api.timezonedb.com/v2.1/get-time-zone?key=%s&format=json&lat=%.2f&lng=%.2f&by=position",
           apiKeyTimezone, latitude, longitude);
  
  LOG_DEBUG_FMT("Timezone API URL: %s", path);

  int attempts = 0;
  bool success = false;

  while (attempts < maxAttemptsTimes && !success) {
    if (http.begin(client, path)) {
      LOG_DEBUG_FMT("Timezone API attempt %d/%d", attempts + 1, maxAttemptsTimes);
      // No session cache here: this runs only at DST boundaries, so any ticket
      // would always be long expired by the time it was needed.
      int httpCode = http.GET();  // Send the request

      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        LOG_VERBOSE_FMT("Timezone API response: %s", payload.c_str());

        StaticJsonDocument<Buffer::JSON_TIMEZONE_SIZE> doc;  // Timezone API response: status, offset, zoneStart, zoneEnd, cityName
        DeserializationError error = deserializeJson(doc, payload);
        if (!error) {
          LOG_VERBOSE_F("Timezone JSON deserialization succeeded");
          JsonObject root = doc.as<JsonObject>();

          const char* status = root["status"];
          if (status != nullptr && strcmp(status, "OK") == 0) {
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
          LOG_ERROR_FMT("Timezone JSON deserialization failed: %s",
                        error.c_str());
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
  time_t epochTime = timeClient.getEpochTime();
  struct tm tmValue;
  if (gmtime_r(&epochTime, &tmValue) == nullptr) {
    return;
  }

  char tape[6];
  snprintf(tape, sizeof(tape), "%02d:%02d", tmValue.tm_hour, tmValue.tm_min);
  drawString(String(tape));
}

void printDateToScreen() {
  time_t epochTime = timeClient.getEpochTime();
  // Use re-entrant conversion and a fixed buffer to avoid extra String churn.
  struct tm tmValue;
  if (gmtime_r(&epochTime, &tmValue) == nullptr) {
    return;
  }
  char tape[6];
  snprintf(tape, sizeof(tape), "%02d/%02d", tmValue.tm_mday, tmValue.tm_mon + 1);
  drawString(String(tape));
}

void printDayToScreen() {
  drawString(String(getDayOfWeek(timeClient.getDay())));
}

void printCityToScreen() {
  displayTextInSetup(city_name);
}

void ntp_init() {
  LOG_INFO_F("Initializing NTP client...");
  timeClient.begin();
  getTimezone();
  timeClient.update();
  String currentTime = timeClient.getFormattedTime();
  LOG_INFO_FMT("NTP synchronized, current time: %s", currentTime.c_str());
  printCityToScreen();
}
