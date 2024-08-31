#pragma once

#include <WiFiClientSecureBearSSL.h>
//#include "global.h"

float data_USD_value = 0.0;
float data_EUR_value = 0.0;

float readCurrency(String path) {
  if (Serial) Serial.println(F("Start get Currency"));

  float data_ILS_value = 0.0;

  //std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
  BearSSL::WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;  //Declare an object of class HTTPClient
  http.setTimeout(1500); // Set the timeout
  
  if (Serial) Serial.println(path);

  if (!http.begin(client, path)) {
    if (Serial) Serial.println(F("Failed to begin HTTP connection"));
    return 0.0;
  }

  // Set the Bearer token
  String authorizationHeader = "Bearer " + bearerTokenCurrency;
  http.setAuthorization("");
  http.addHeader("Authorization", authorizationHeader);
  http.addHeader("Content-Type", "application/json");

  int httpCode = http.GET();  //Send the request
  if (httpCode == HTTP_CODE_OK) {  //Check the returning code

    String payload = http.getString();  //Get the request response payload

    if (Serial) 
    {
      Serial.println(F("payload: "));
      Serial.println(payload);
      Serial.println(F("payload length: "));
      Serial.println(payload.length());
    }

    JsonDocument doc;

    DeserializationError error = deserializeJson(doc, payload);

    if (error) {
      if (Serial)
      {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
      }
      return 0.0;
    }
    data_ILS_value = doc["state"];

  } else {
    if (Serial) Serial.println(F("No Currency - ") + String(httpCode, DEC));
    //drawString("No Currency - " + String(httpCode, DEC), 0);
  }

  http.end();  //Close connection

  return data_ILS_value;
}

void printCurrencyUSDToScreen() {
  if (data_USD_value > 0) {
    String tape = "$ " + String(data_USD_value, 2);
    drawString(tape);
  }
}

void printCurrencyEURToScreen() {
  if (data_EUR_value > 0) {
    String tape = "\x84 " + String(data_EUR_value, 2);
    drawString(tape);
  }
}

void currency_init() {

  float tmp_data_usd = readCurrency(pathCurrencyUSD);

  if (tmp_data_usd > 0) data_USD_value = tmp_data_usd;
  
  yield();

  float tmp_data_eur = readCurrency(pathCurrencyEUR);

  if (tmp_data_eur > 0) data_EUR_value = tmp_data_eur;
}
