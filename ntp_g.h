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
  Serial.println("Start get timezone");

  Serial.println(zoneEnd);

  Serial.println(timeNow);


  if (zoneEnd > timeNow) {
    time_t untilTimeMove = zoneEnd - timeNow;

    int daysUntilTimeMove = untilTimeMove / 86400;
    Serial.print("Days before hour move: ");
    Serial.println(daysUntilTimeMove);
    Serial.println("Don't need to update timezone");
    return;
  }



  //std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);

  BearSSL::WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;  //Declare an object of class HTTPClient

  String path = "https://api.timezonedb.com/v2.1/get-time-zone?key=" + apiKeyTimezone + "&format=json&lat=" + String(latitude, 2) + "&lng=" + String(longitude, 2) + "&by=position";


  Serial.println(path);

  //http.begin(path);//, "17 11 5e 7c a9 de 30 4b 80 b0 23 c8 70 25 e5 b6 99 5e 4e 34");  //Specify request destination
  http.begin(client, path);   //, "17 11 5e 7c a9 de 30 4b 80 b0 23 c8 70 25 e5 b6 99 5e 4e 34");  //Specify request destination
  int httpCode = http.GET();  //Send the request

  if (httpCode == HTTP_CODE_OK) {  //Check the returning code

    String payload = http.getString();  //Get the request response payload

    Serial.println(payload);

    DynamicJsonDocument doc(payload.length() * 2);
    DeserializationError error = deserializeJson(doc, payload);

    // Test if parsing succeeds.
    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.c_str());
      return;

    } else {

      Serial.println(F("Deserialization succeeded"));
      JsonObject root = doc.as<JsonObject>();

      const char* status = root["status"];  // "OK"

      if (strcmp(status, "OK") == 0) {
        offset = (int)root["gmtOffset"];  // 10800

        timeClient.setTimeOffset(offset);

        zoneStart = (unsigned long)root["zoneStart"];
        zoneEnd = (unsigned long)root["zoneEnd"];

        const char* name_ct = root["cityName"];

        city_name = String(name_ct);


        Serial.print("offset: ");
        Serial.println(offset);
      }
    }
  } else {
    Serial.print("httpCode ");
    Serial.println(httpCode);

    //drawString("No timezone 2", 0);
    ESP.restart();
  }

  http.end();  //Close connection
  return;
}

void printTimeToScreen() {
  //Serial.println("Time: " + timeClient.getFormattedTime());
  String tape = timeClient.getFormattedTime().substring(0, 5);  //formattedTime("%H:%M");//.substring(0, 5);
  drawString(tape, 0);
}

void printDateToScreen() {
  time_t epochTime = timeClient.getEpochTime();
  struct tm* ptm = gmtime((time_t*)&epochTime);
  String tape = getNumberWithZerro(ptm->tm_mday) + "/" + getNumberWithZerro(ptm->tm_mon + 1);
  /*String tape = timeClient.formattedTime("%d/%m");*/
  int mv = 5 - tape.length();
  drawString(tape, mv);
}

void printDayToScreen() {
  String tape = daysOfTheWeek[timeClient.getDay()];
  drawString(tape, 2);
}

void printCityToScreen() {
  drawString(city_name, 0);
}

void ntp_init() {
  timeClient.begin();
  getTimezone();
  timeClient.update();

  printCityToScreen();
}