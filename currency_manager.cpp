#include "currency_manager.h"

// Define global variables
float data_USD_value = 0.0;
float data_EUR_value = 0.0;

// Function to read currency data from a given path
float readCurrency(String path) {
    if (Serial) Serial.println(F("Start get Currency"));

    float data_ILS_value = 0.0;

    BearSSL::WiFiClientSecure client;
    client.setInsecure();  // Disable SSL certificate verification

    HTTPClient http;  // Declare HTTP client
    http.setTimeout(1500);  // Set timeout for HTTP requests

    if (Serial) Serial.println(path);

    if (!http.begin(client, path)) {
        if (Serial) Serial.println(F("Failed to begin HTTP connection"));
        return 0.0;
    }

    // Set the Bearer token for authorization
    String authorizationHeader = F("Bearer ") + bearerTokenCurrency;
    http.addHeader(F("Authorization"), authorizationHeader);
    http.addHeader(F("Content-Type"), F("application/json"));

    // Send GET request
    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK) {  // Check for successful response
        String payload = http.getString();  // Get the response payload

        if (Serial) {
            Serial.println(F("payload: "));
            Serial.println(payload);
            Serial.println(F("payload length: "));
            Serial.println(payload.length());
        }

        DynamicJsonDocument doc(1024);  // Adjust size as necessary
        DeserializationError error = deserializeJson(doc, payload);

        if (error) {
            if (Serial) {
                Serial.print(F("deserializeJson() failed: "));
                Serial.println(error.f_str());
            }
            return 0.0;
        }
        data_ILS_value = doc[F("state")];  // Extract currency value from JSON
    } else {
        if (Serial) Serial.println(F("No Currency - ") + String(httpCode, DEC));
    }

    http.end();  // Close the HTTP connection
    return data_ILS_value;
}

// Function to display USD currency on the screen
void printCurrencyUSDToScreen() {
    if (data_USD_value > 0) {
        String tape = F("$ ") + String(data_USD_value, 2);
        drawString(tape);
    }
}

// Function to display EUR currency on the screen
void printCurrencyEURToScreen() {
    if (data_EUR_value > 0) {
        String tape = F("\x84 ") + String(data_EUR_value, 2);
        drawString(tape);
    }
}

// Function to initialize and fetch USD and EUR currency data
void currency_init() {
    // Fetch USD currency value
    float tmp_data_usd = readCurrency(pathCurrencyUSD);
    if (tmp_data_usd > 0) data_USD_value = tmp_data_usd;
  
    yield();  // Allow system tasks

    // Fetch EUR currency value
    float tmp_data_eur = readCurrency(pathCurrencyEUR);
    if (tmp_data_eur > 0) data_EUR_value = tmp_data_eur;
}
