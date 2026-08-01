#include "weather_manager.h"
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <ArduinoJson.h>
#include "Secret.h"
#include "location_manager.h"
#include "constants.h"
#include "logger.h"

// Root CA for api.openweathermap.org. The site's certificate currently chains
// through Sectigo intermediates up to this self-signed USERTrust root (valid
// until 2038), so pinning the root here survives normal leaf/intermediate
// rotation instead of breaking every renewal.
static const char OPENWEATHERMAP_ROOT_CA[] PROGMEM = R"CERT(
-----BEGIN CERTIFICATE-----
MIIF3jCCA8agAwIBAgIQAf1tMPyjylGoG7xkDjUDLTANBgkqhkiG9w0BAQwFADCB
iDELMAkGA1UEBhMCVVMxEzARBgNVBAgTCk5ldyBKZXJzZXkxFDASBgNVBAcTC0pl
cnNleSBDaXR5MR4wHAYDVQQKExVUaGUgVVNFUlRSVVNUIE5ldHdvcmsxLjAsBgNV
BAMTJVVTRVJUcnVzdCBSU0EgQ2VydGlmaWNhdGlvbiBBdXRob3JpdHkwHhcNMTAw
MjAxMDAwMDAwWhcNMzgwMTE4MjM1OTU5WjCBiDELMAkGA1UEBhMCVVMxEzARBgNV
BAgTCk5ldyBKZXJzZXkxFDASBgNVBAcTC0plcnNleSBDaXR5MR4wHAYDVQQKExVU
aGUgVVNFUlRSVVNUIE5ldHdvcmsxLjAsBgNVBAMTJVVTRVJUcnVzdCBSU0EgQ2Vy
dGlmaWNhdGlvbiBBdXRob3JpdHkwggIiMA0GCSqGSIb3DQEBAQUAA4ICDwAwggIK
AoICAQCAEmUXNg7D2wiz0KxXDXbtzSfTTK1Qg2HiqiBNCS1kCdzOiZ/MPans9s/B
3PHTsdZ7NygRK0faOca8Ohm0X6a9fZ2jY0K2dvKpOyuR+OJv0OwWIJAJPuLodMkY
tJHUYmTbf6MG8YgYapAiPLz+E/CHFHv25B+O1ORRxhFnRghRy4YUVD+8M/5+bJz/
Fp0YvVGONaanZshyZ9shZrHUm3gDwFA66Mzw3LyeTP6vBZY1H1dat//O+T23LLb2
VN3I5xI6Ta5MirdcmrS3ID3KfyI0rn47aGYBROcBTkZTmzNg95S+UzeQc0PzMsNT
79uq/nROacdrjGCT3sTHDN/hMq7MkztReJVni+49Vv4M0GkPGw/zJSZrM233bkf6
c0Plfg6lZrEpfDKEY1WJxA3Bk1QwGROs0303p+tdOmw1XNtB1xLaqUkL39iAigmT
Yo61Zs8liM2EuLE/pDkP2QKe6xJMlXzzawWpXhaDzLhn4ugTncxbgtNMs+1b/97l
c6wjOy0AvzVVdAlJ2ElYGn+SNuZRkg7zJn0cTRe8yexDJtC/QV9AqURE9JnnV4ee
UB9XVKg+/XRjL7FQZQnmWEIuQxpMtPAlR1n6BB6T1CZGSlCBst6+eLf8ZxXhyVeE
Hg9j1uliutZfVS7qXMYoCAQlObgOK6nyTJccBz8NUvXt7y+CDwIDAQABo0IwQDAd
BgNVHQ4EFgQUU3m/WqorSs9UgOHYm8Cd8rIDZsswDgYDVR0PAQH/BAQDAgEGMA8G
A1UdEwEB/wQFMAMBAf8wDQYJKoZIhvcNAQEMBQADggIBAFzUfA3P9wF9QZllDHPF
Up/L+M+ZBn8b2kMVn54CVVeWFPFSPCeHlCjtHzoBN6J2/FNQwISbxmtOuowhT6KO
VWKR82kV2LyI48SqC/3vqOlLVSoGIG1VeCkZ7l8wXEskEVX/JJpuXior7gtNn3/3
ATiUFJVDBwn7YKnuHKsSjKCaXqeYalltiz8I+8jRRa8YFWSQEg9zKC7F4iRO/Fjs
8PRF/iKz6y+O0tlFYQXBl2+odnKPi4w2r78NBc5xjeambx9spnFixdjQg3IM8WcR
iQycE0xyNN+81XHfqnHd4blsjDwSXWXavVcStkNr/+XeTWYRUc+ZruwXtuhxkYze
Sf7dNXGiFSeUHM9h4ya7b6NnJSFd5t0dCy5oGzuCr+yDZ4XUmFF0sbmZgIn/f3gZ
XHlKYC6SQK5MNyosycdiyA5d9zZbyuAlJQG03RoHnHcAP9Dc1ew91Pq7P8yF1m9/
qS3fuQL39ZeatTXaw2ewh0qpKJ4jjv9cJ2vhsE/zB+4ALtRZh8tSQZXq9EfX7mRB
VXyNWQKV3WKdwrnuWih0hKWbt5DHDAff9Yk2dDLWKMGwsAvgnEzDHNb842m1R0aB
L6KCq9NjRHDEjf8tM7qtj3u1cIiuPhnPQCjY/MiQu12ZIvVS5ljFH4gxQ+6IHdfG
jjxDah2nGN59PRbxYvnKkKj9
-----END CERTIFICATE-----
)CERT";

WeatherManager::WeatherManager()
{
    // Initialize weather data members here, if necessary
    temperature = 0;
    temp_max = 0;
    pressure = 0;
    humidity = 0;
    main_ext_humidity = 0;
    description_weather = "";
}

// Function to read weather data from the OpenWeather API
void WeatherManager::readWeather() {
  LOG_INFO_F("Fetching weather data from OpenWeatherMap...");

  int maxAttempts = Retry::MAX_ATTEMPTS_WEATHER;

  // BearSSL needs a real system clock to validate the certificate's
  // notBefore/notAfter dates; this is a no-op once NTP has already synced.
  setClock();

  BearSSL::WiFiClientSecure client;
  BearSSL::X509List rootCA(OPENWEATHERMAP_ROOT_CA);
  client.setTrustAnchors(&rootCA);
  LOG_DEBUG_F("Weather client configured with certificate validation (OpenWeatherMap root CA)");
  HTTPClient http;
  http.setTimeout(Timing::HTTP_TIMEOUT_MS);

  // Optimize URL construction to avoid multiple String concatenations
  char path[Buffer::PATH_BUFFER_SIZE];
  snprintf(path, sizeof(path), 
           "https://api.openweathermap.org/data/3.0/onecall?lat=%.2f&lon=%.2f&units=metric&exclude=minutely,hourly,daily,alerts&appid=%s&lang=%s",
           latitude, longitude, appidWeather, lang_weather.c_str());

  LOG_DEBUG_FMT("Weather API URL: %s", path);

  int attempts = 0;
  bool success = false;

  while (attempts < maxAttempts && !success) {
    if (http.begin(client, path)) {
      LOG_DEBUG_FMT("Weather API attempt %d/%d", attempts + 1, maxAttempts);
      int httpCode = http.GET();  // Send the request

      if (httpCode == HTTP_CODE_OK) {  // Check the returning code
        String payload = http.getString();  // Get the request response payload
        LOG_VERBOSE_FMT("Weather API response: %s", payload.c_str());

        StaticJsonDocument<Buffer::JSON_WEATHER_SIZE> doc;  // Weather API response with current weather data
        DeserializationError error = deserializeJson(doc, payload);

        // Test if parsing succeeds
        if (!error) {
          JsonObject current = doc[F("current")];
          // Bug fix #6: timezone_offset can be negative (e.g. UTC-5 = -18000).
          // Store as int; sunrise/sunset are full epoch values so use unsigned long.
          int timezone_offset = (int)doc[F("timezone_offset")];
          sunrise = (unsigned long)((long)current[F("sunrise")] + timezone_offset);
          sunset  = (unsigned long)((long)current[F("sunset")]  + timezone_offset);

          temperature = (int)floor((double)current[F("temp")] + 0.5);
          temp_max = (int)floor((double)current[F("feels_like")] + 0.5);
          pressure = (int)((double)current[F("pressure")] * 0.75006375541921);  // Convert pressure to mmHg
          main_ext_humidity = current[F("humidity")];

          JsonObject weather = current[F("weather")][0];
          description_weather = String(weather[F("description")]);
          description_weather.toUpperCase();

          LOG_INFO_FMT("Weather updated: %d C, %u%%, %dmm", temperature,
                       main_ext_humidity, pressure);
          LOG_DEBUG_FMT("Feels like: %d C", temp_max);
          LOG_DEBUG_FMT("Description: %s", description_weather.c_str());

          success = true;  // Set success flag
          // Bug fix #7: removed redundant `maxAttempts = 1` — the while
          // condition already checks `!success`, so the loop exits correctly.
        } else {
          LOG_ERROR_FMT("Weather JSON deserialization failed: %s",
                        error.c_str());
        }
      } else {
        LOG_WARNING_FMT("Weather API HTTP error: %d", httpCode);
      }

    } else {
      LOG_ERROR_F("Failed to begin weather HTTP connection");
    }

    http.end();

    if (!success) {
      attempts++;
      if (attempts < maxAttempts) {
        LOG_WARNING_FMT("Retrying weather request (%d/%d)...", attempts, maxAttempts);
        delay(Timing::RETRY_DELAY_MS);
        yield(); // Allow system tasks
      } else {
        LOG_ERROR_FMT("Failed to get weather data after %d attempts", maxAttempts);
      }
    }
  }
}

// Function to print temperature on the screen
void WeatherManager::printWeatherToScreen() const{
  char tape[12];
  snprintf(tape, sizeof(tape), "%d%cC", temperature, getGradValue());
  drawString(String(tape));
}

// Function to print feels-like temperature on the screen
void WeatherManager::printMaxTempToScreen() const{
  // Very short format to fit on display: "~25°C" (tilde ~ means "feels like")
  char tape[12];
  snprintf(tape, sizeof(tape), "~%d%cC", temp_max, getGradValue());
  drawString(String(tape));
}

// Function to print pressure on the screen
void WeatherManager::printPressureToScreen() const{
  char tape[12];
  snprintf(tape, sizeof(tape), "%dmm", pressure);
  drawString(String(tape));
}

// Function to print humidity on the screen
void WeatherManager::printHumidityToScreen() const{
  char tape[8];
  snprintf(tape, sizeof(tape), "%u%%", main_ext_humidity);
  drawString(String(tape));
}

// Function to print weather description on the screen
void WeatherManager::printDescriptionWeatherToScreen() const {  
  drawString(description_weather);
}
