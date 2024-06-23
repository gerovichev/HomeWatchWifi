#pragma once

#include <WiFiClientSecureBearSSL.h>
//#include "global.h"

float data_USD_value = 0.0;
float data_EUR_value = 0.0;

float readCurrency(String path) {
  Serial.println("Start get Currency");

  float data_ILS_value = 0.0;

  //std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
  BearSSL::WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;  //Declare an object of class HTTPClient

  Serial.println(path);
  Serial.println("currency 1");
  if (!http.begin(client, path)) {
    Serial.println("Failed to begin HTTP connection");
    return 0.0;
  }

  Serial.println("currency 2");
    // Set the Bearer token
  String authorizationHeader = "Bearer " + bearerTokenCurrency;
  Serial.println("currency 3");
  http.setAuthorization("");
  http.addHeader("Authorization", authorizationHeader);
  http.addHeader("Content-Type", "application/json");
  Serial.println("currency 4");

  int httpCode = http.GET();  //Send the request
  Serial.println("currency 5");
  if (httpCode == HTTP_CODE_OK) {  //Check the returning code
    Serial.println("currency 6");
    String payload = http.getString();  //Get the request response payload

    Serial.println("payload: ");
    Serial.println(payload);
    Serial.println("payload length: ");
    Serial.println(payload.length());

    JsonDocument doc;

    DeserializationError error = deserializeJson(doc, payload);

    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      return 0.0;
    }
    data_ILS_value = doc["state"];

  } else {
    Serial.println("No Currency - " + String(httpCode, DEC));
    //drawString("No Currency - " + String(httpCode, DEC), 0);
  }

  http.end();  //Close connection

  return data_ILS_value;
}

void printCurrencyUSDToScreen() {
  if (data_USD_value > 0) {
    String tape = "$ " + String(data_USD_value, 2);
    int mv = 5 - tape.length();
    drawString(tape, mv);
  }
}

void printCurrencyEURToScreen() {
  if (data_EUR_value > 0) {
    String tape = "\x84 " + String(data_EUR_value, 2);
    int mv = 5 - tape.length();
    drawString(tape, mv);
  }
}

void currency_init() {

  float tmp_data_usd = readCurrency(pathCurrencyUSD);

  if (tmp_data_usd > 0) data_USD_value = tmp_data_usd;

  float tmp_data_eur = readCurrency(pathCurrencyEUR);

  if (tmp_data_eur > 0) data_EUR_value = tmp_data_eur;
}
