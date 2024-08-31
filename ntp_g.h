#pragma once

#include <ArduinoJson.h>
#include <WiFiClientSecureBearSSL.h>
#include <NTPClient.h>


WiFiUDP ntpUDP;

NTPClient timeClient(ntpUDP);  //, "pool.ntp.org");

time_t zoneStart;
time_t zoneEnd;
time_t timeNow;

int offset;

String city_name;

String getNumberWithZerro(int dig);

void getTimezone() {
  if (Serial) 
  {
    Serial.println("Start get timezone");

    Serial.println(zoneEnd);
    Serial.println(timeNow);
  }

  if (zoneEnd > timeNow) {
    time_t untilTimeMove = zoneEnd - timeNow;
    int daysUntilTimeMove = untilTimeMove / 86400;
    if (Serial) 
    {
      Serial.print("Days before hour move: ");
      Serial.println(daysUntilTimeMove);
      Serial.println("Don't need to update timezone");
    }
    return;
  }

  BearSSL::WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;

  String path = "https://api.timezonedb.com/v2.1/get-time-zone?key=" + apiKeyTimezone + "&format=json&lat=" + String(latitude, 2) + "&lng=" + String(longitude, 2) + "&by=position";

  Serial.println(path);

  int attempts = 0;
  const int maxAttempts = 3;
  bool success = false;

  while (attempts < maxAttempts && !success) {
    if (http.begin(client, path)) {
      if (Serial) Serial.println("Start timezone attempt " + String(attempts + 1));
      int httpCode = http.GET();  // Send the request

      if (httpCode == HTTP_CODE_OK) {       // Check the returning code
        String payload = http.getString();  // Get the request response payload
        if (Serial) Serial.println(payload);

        JsonDocument doc;  
        DeserializationError error = deserializeJson(doc, payload);
        // Test if parsing succeeds
        if (!error) {
          if (Serial) Serial.println(F("Deserialization succeeded"));
          JsonObject root = doc.as<JsonObject>();

          const char* status = root["status"];  // "OK"

          if (strcmp(status, "OK") == 0) {
            offset = (int)root["gmtOffset"];  // 10800
            timeClient.setTimeOffset(offset);

            zoneStart = (unsigned long)root["zoneStart"];
            zoneEnd = (unsigned long)root["zoneEnd"];

            const char* name_ct = root["cityName"];
            city_name = String(name_ct);

            if (Serial) Serial.println(F("offset: ") + String(offset));

            success = true;  // Set success flag
          }
        } else {
          if (Serial) Serial.println(F("deserializeJson() failed: ") + String(error.c_str()));
        }
      } else {
        if (Serial) Serial.println(F("No timezone response: ") + String(httpCode));
      }

      http.end();  // Close connection
    } else {
      if (Serial) Serial.println(F("Failed to begin HTTP connection"));
    }

    if (!success) {
      attempts++;
      if (attempts < maxAttempts) {
        if (Serial) Serial.println(F("Retrying... (") + String(attempts) + F("/") + String(maxAttempts) + F(")"));
        delay(2000);  // Wait for 2 seconds before retrying
      } else {
        if (Serial) Serial.println(F("Failed to get timezone data after ") + String(maxAttempts) + F(" attempts."));
      }
    }
  }
}


void printTimeToScreen() {
  //Serial.println("Time: " + timeClient.getFormattedTime());
  String tape = timeClient.getFormattedTime().substring(0, 5);  //formattedTime("%H:%M");//.substring(0, 5);
  drawString(tape);
}

void printDateToScreen() {
  time_t epochTime = timeClient.getEpochTime();
  struct tm* ptm = gmtime((time_t*)&epochTime);
  String tape = getNumberWithZerro(ptm->tm_mday) + F("/") + getNumberWithZerro(ptm->tm_mon + 1);
  drawString(tape);
}

void printDayToScreen() {
  String tape = daysOfTheWeek[timeClient.getDay()];
  drawString(tape);
}

void printCityToScreen() {
  drawString(city_name);
}

void ntp_init() {
  timeClient.begin();
  getTimezone();
  timeClient.update();

  printCityToScreen();
}