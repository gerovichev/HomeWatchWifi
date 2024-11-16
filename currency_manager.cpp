#include "currency_manager.h"
#include <WiFiClientSecureBearSSL.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>

const int HTTP_TIMEOUT = 1500;  // Timeout for HTTP requests in milliseconds

// Define global variables
float data_USD_value = 0.0;
float data_EUR_value = 0.0;

// Helper function to set up the HTTP client
bool setupHttpClient(HTTPClient &http, BearSSL::WiFiClientSecure &client, const String &path) {
    client.setInsecure();  // Disable SSL certificate verification
    http.setTimeout(HTTP_TIMEOUT);  // Set timeout for HTTP requests

    if (!http.begin(client, path)) {
        if (Serial) Serial.println(F("Failed to begin HTTP connection"));
        return false;
    }

    // Set the Bearer token for authorization
    http.addHeader(F("Authorization"), F("Bearer ") + bearerTokenCurrency);
    http.addHeader(F("Content-Type"), F("application/json"));
    return true;
}

// Helper function to handle the HTTP response
float handleHttpResponse(HTTPClient &http) {
    int httpCode = http.GET();
    if (httpCode != HTTP_CODE_OK) {
        if (Serial) Serial.println(F("HTTP request failed with code: ") + String(httpCode, DEC));
        return 0.0;
    }

    String payload = http.getString();  // Get the response payload
    if (Serial) {
        Serial.println(F("payload: "));
        Serial.println(payload);
        Serial.println(F("payload length: "));
        Serial.println(payload.length());
    }

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, payload);
    if (error) {
        if (Serial) {
            Serial.print(F("deserializeJson() failed: "));
            Serial.println(error.f_str());
        }
        return 0.0;
    }

    return doc[F("state")];  // Extract currency value from JSON
}

// Function to read currency data from a given path
float readCurrency(String path) {
    if (Serial) Serial.println(F("Start get Currency"));

    BearSSL::WiFiClientSecure client;
    HTTPClient http;

    if (Serial) Serial.println(path);

    if (!setupHttpClient(http, client, path)) {
        return 0.0;
    }

    float currencyValue = handleHttpResponse(http);
    http.end();  // Close the HTTP connection
    return currencyValue;
}

// Function to display USD currency on the screen
void printCurrencyUSDToScreen() {
    if (data_USD_value > 0) {
        drawString(F("$ ") + String(data_USD_value, 2));
    }
}

// Function to display EUR currency on the screen
void printCurrencyEURToScreen() {
    if (data_EUR_value > 0) {
        drawString(F("\x84 ") + String(data_EUR_value, 2));
    }
}

// Function to initialize and fetch USD and EUR currency data
void currency_init() {
    // Fetch USD currency value
    if (float tmp_data_usd = readCurrency(pathCurrencyUSD); tmp_data_usd > 0) {
        data_USD_value = tmp_data_usd;
    }

    yield();  // Allow system tasks

    // Fetch EUR currency value
    if (float tmp_data_eur = readCurrency(pathCurrencyEUR); tmp_data_eur > 0) {
        data_EUR_value = tmp_data_eur;
    }
}