#pragma once

#include <WiFiClientSecureBearSSL.h>
//#include "global.h"

float data_USD_value;
float data_EUR_value;

time_t timeLastCallCurrency;

float tolerance = 0.0001; // Define a tolerance for comparisons

float readCurrency(String path) {
  Serial.println("Start get Currency");

  float data_ILS_value;

  if ((timeLastCallCurrency + 1 * 60 * 60 < timeNow) || (fabs(data_ILS_value - 0.0) < tolerance)) {

    timeLastCallCurrency = timeNow;

    //std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
    BearSSL::WiFiClientSecure client;
    client.setInsecure();

    HTTPClient http;  //Declare an object of class HTTPClient

    Serial.println(path);

    http.begin(client, path);
    // Set the Bearer token
    String authorizationHeader = "Bearer " + bearerTokenCurrency;
    http.setAuthorization("");
    http.addHeader("Authorization", authorizationHeader);
    http.addHeader("Content-Type", "application/json");

    int httpCode = http.GET();  //Send the request

    if (httpCode == HTTP_CODE_OK) {  //Check the returning code

      String payload = http.getString();  //Get the request response payload

      Serial.println("payload: ");
      Serial.println(payload);
      Serial.println("payload length: ");
      Serial.println(payload.length());

      StaticJsonDocument<800> doc;

      DeserializationError error = deserializeJson(doc, payload);

      if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return 0.0;
      }
      data_ILS_value = doc["state"];

    } else {
      drawString("No Currency - " + String(httpCode, DEC), 0);
    }

    http.end();  //Close connection
  }

  return data_ILS_value;
}

void printCurrencyUSDToScreen() {

  String tape = "$ " + String(data_USD_value, 2);
  int mv = 5 - tape.length();
  drawString(tape, mv);
}

void printCurrencyEURToScreen() {

  String tape = "\x84 " + String(data_EUR_value, 2);
  int mv = 5 - tape.length();
  drawString(tape, mv);
}

void currency_init() {
  data_USD_value = readCurrency(pathCurrencyUSD);

  data_EUR_value = readCurrency(pathCurrencyEUR);
}
