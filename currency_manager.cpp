// CurrencyManager.cpp
#include "currency_manager.h"
#include "clock.h"
#include "constants.h"
#include "logger.h"
#include "location_manager.h"
#include "root_certs.h"

CurrencyManager::CurrencyManager()
    : bearerTokenCurrency(confBearerTokenCurrency),
      pathCurrencyUSD(confPathCurrencyUSD),
      pathCurrencyEUR(confPathCurrencyEUR), pathCryptoBTC(confPathCryptoBTC),
      dataUSDValue(0.0), dataEURValue(0.0), dataBTCValue(0.0) {
  // const char* pointers are assigned directly - no String copies, saves RAM
}

void CurrencyManager::initialize() {
  if (float tmpDataUSD = readCurrency(pathCurrencyUSD); tmpDataUSD > 0) {
    dataUSDValue = tmpDataUSD;
  }

  yield(); // Allow system tasks

  if (float tmpDataEUR = readCurrency(pathCurrencyEUR); tmpDataEUR > 0) {
    dataEURValue = tmpDataEUR;
  }

  yield();

  if (float tmpDataBTC = readCrypto(pathCryptoBTC); tmpDataBTC > 0) {
    dataBTCValue = tmpDataBTC;
  }
}

void CurrencyManager::displayUSDToScreen() {
  if (dataUSDValue > 0) {
    drawString(F("$ ") + String(dataUSDValue, 2));
  } else {
    // Bug fix #8: when data hasn't loaded yet, skip this slot so the display
    // advances immediately instead of freezing on blank content.
    Clock::getInstance().skipCurrentDisplay();
  }
}

void CurrencyManager::displayEURToScreen() {
  if (dataEURValue > 0) {
    drawString(F("\x84 ") + String(dataEURValue, 2));
  } else {
    Clock::getInstance().skipCurrentDisplay();
  }
}

void CurrencyManager::displayBTCToScreen() {
  if (dataBTCValue > 0) {
    drawString(
        F("B$ ") +
        String(dataBTCValue, 0));
  } else {
    Clock::getInstance().skipCurrentDisplay();
  }
}

// Unified HTTP fetch with retry — replaces duplicated readCurrency/readCrypto (~180 lines saved)
float CurrencyManager::fetchWithRetry(const char* path, const char* token, ResponseParser parser) {
    // Check WiFi connection before attempting request
    if (WiFi.status() != WL_CONNECTED) {
        LOG_ERROR_F("WiFi not connected, cannot fetch data");
        return 0.0;
    }

    int maxAttempts = Retry::MAX_ATTEMPTS_CURRENCY;
    LOG_DEBUG_FMT("API path: %s", path);

    // BearSSL needs a real system clock to validate the certificate's
    // notBefore/notAfter dates; this is a no-op once NTP has already synced.
    setClock();

    int attempts = 0;
    bool success = false;
    float resultValue = 0.0;

    while (attempts < maxAttempts && !success) {
        // Verify WiFi is still connected before each attempt
        if (WiFi.status() != WL_CONNECTED) {
            LOG_ERROR_FMT("WiFi disconnected during fetch (attempt %d)", attempts + 1);
            break;
        }
        
        yield(); // Allow system tasks before creating client
        
        // Create fresh client for each attempt to avoid BearSSL reuse issues
        BearSSL::WiFiClientSecure client;
        BearSSL::X509List rootCA(ISRG_ROOT_X1);
        client.setTrustAnchors(&rootCA);
        HTTPClient http;
        http.setTimeout(Timing::HTTP_TIMEOUT_CURRENCY_MS);
        
        // Add reconnect delay for retries
        if (attempts > 0) {
            delay(500); // Small delay before retry
            yield();
        }

        LOG_DEBUG_FMT("Attempting HTTP connection (attempt %d/%d)...", attempts + 1, maxAttempts);
        bool connectionStarted = http.begin(client, path);
        
        if (connectionStarted) {
            String authHeader = F("Bearer ") + String(token);
            http.addHeader(F("Authorization"), authHeader);
            http.addHeader(F("Content-Type"), F("application/json"));
            
            LOG_DEBUG_F("HTTP connection started, sending GET request...");
            yield(); // Allow system tasks before sending request
            
            // Perform HTTP GET
            unsigned long startTime = millis();
            int httpCode = http.GET();
            unsigned long elapsed = millis() - startTime;
            
            LOG_DEBUG_FMT("http.GET() completed in %lu ms, code: %d", elapsed, httpCode);
            
            if (httpCode == HTTP_CODE_OK) {
                String payload = http.getString();
                LOG_VERBOSE_FMT("API payload length: %u", payload.length());

                // Use the specific parser callback
                resultValue = parser(payload);
            } else {
                if (httpCode == -1) {
                    LOG_WARNING_FMT("HTTP request failed (code %d): connection failed - SSL handshake likely failed", httpCode);
                    LOG_DEBUG_FMT("WiFi status: %s", WiFi.status() == WL_CONNECTED ? "Connected" : "Disconnected");
                } else if (httpCode < 0) {
                    LOG_WARNING_FMT("HTTP request failed (code %d): HTTP client error", httpCode);
                } else if (httpCode >= 400 && httpCode < 500) {
                    LOG_WARNING_FMT("HTTP request failed (code %d): client error - check auth", httpCode);
                    String errorBody = http.getString();
                    if (errorBody.length() > 0) {
                        LOG_DEBUG_FMT("Error response body: %s",
                                      errorBody.substring(0, min(200, (int)errorBody.length())).c_str());
                    }
                } else if (httpCode >= 500) {
                    LOG_WARNING_FMT("HTTP request failed (code %d): server error", httpCode);
                } else {
                    LOG_WARNING_FMT("HTTP request failed (code %d)", httpCode);
                }
            }
            
            http.end();
            client.stop(); // Explicitly stop the client
            yield(); // Allow cleanup to complete
            
            if (resultValue > 0.0) {
                success = true;
                LOG_DEBUG_FMT("Data retrieved successfully: %.2f", resultValue);
            } else {
                LOG_WARNING_FMT("Failed to retrieve data (attempt %d/%d)", attempts + 1, maxAttempts);
            }
        } else {
            LOG_ERROR_FMT("Failed to begin HTTP connection (attempt %d/%d)", attempts + 1, maxAttempts);
            LOG_DEBUG_FMT("WiFi status: %d, RSSI: %d", WiFi.status(), WiFi.RSSI());
            client.stop();
            yield();
        }

        if (!success) {
            attempts++;
            if (attempts < maxAttempts) {
                LOG_WARNING_FMT("Retrying request (%d/%d)...", attempts, maxAttempts);
                delay(Timing::RETRY_DELAY_MS);
                yield(); // Allow system tasks
            } else {
                LOG_ERROR_FMT("Failed to get data after %d attempts", maxAttempts);
            }
        }
    }

    return resultValue;
}

// Parse Home Assistant currency response: {"state": "3.65", ...}
float CurrencyManager::parseCurrencyResponse(const String& payload) {
    StaticJsonDocument<Buffer::JSON_CURRENCY_SIZE> doc;
    DeserializationError error = deserializeJson(doc, payload);
    if (error) {
        LOG_ERROR_FMT("parseCurrencyResponse: deserializeJson() failed: %s", error.c_str());
        return 0.0;
    }
    return doc[F("state")];
}

// Convenience wrappers that use the unified fetchWithRetry
float CurrencyManager::readCurrency(const char* path) {
    LOG_DEBUG_F("Fetching currency data...");
    return fetchWithRetry(path, bearerTokenCurrency, parseCurrencyResponse);
}

float CurrencyManager::readCrypto(const char* path) {
    LOG_DEBUG_F("Fetching crypto data...");
    return fetchWithRetry(path, bearerTokenCurrency, parseCurrencyResponse);
}
