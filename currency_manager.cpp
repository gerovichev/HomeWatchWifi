// CurrencyManager.cpp
#include "currency_manager.h"
#include "constants.h"
#include "logger.h"
#include "secure_client.h"

CurrencyManager::CurrencyManager()
    : bearerTokenCurrency(confBearerTokenCurrency),
      bearerTokenCrypto(confBearerTokenCrypto),
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
  }
}

void CurrencyManager::displayEURToScreen() {
  if (dataEURValue > 0) {
    drawString(F("\x84 ") + String(dataEURValue, 2));
  }
}

void CurrencyManager::displayBTCToScreen() {
  if (dataBTCValue > 0) {
    drawString(
        F("B$ ") +
        String(
            dataBTCValue,
            0)); // \x80 is often a bitcoin symbol in custom fonts or just 'B'
  }
}

bool CurrencyManager::setupHttpClient(HTTPClient &http,
                                      BearSSL::WiFiClientSecure &client,
                                      const char *path, const char *token) {
  setupSecureClient(client, "currency API");
  http.setTimeout(Timing::HTTP_TIMEOUT_CURRENCY_MS);

  LOG_DEBUG("Currency API path: " + String(path));

  if (!http.begin(client, path)) {
    LOG_ERROR_F("Failed to begin HTTP connection");
    return false;
  }

  String authHeader = F("Bearer ") + String(token);
  LOG_DEBUG("Currency API Bearer: " + String(authHeader));
  http.addHeader(F("Authorization"), authHeader);
  http.addHeader(F("Content-Type"), F("application/json"));
  return true;
}

float CurrencyManager::handleHttpResponse(HTTPClient &http) {
    LOG_VERBOSE("Calling http.GET()...");
    unsigned long startTime = millis();
    int httpCode = http.GET();
    unsigned long elapsed = millis() - startTime;
    
    LOG_DEBUG("http.GET() completed in " + String(elapsed) + "ms, code: " + String(httpCode));
    
    if (httpCode != HTTP_CODE_OK) {
        // Provide more detailed error information
        String errorMsg = "HTTP request failed with code: " + String(httpCode, DEC);
        if (httpCode == -1) {
            errorMsg += " (Connection failed - SSL handshake likely failed)";
            LOG_DEBUG("Error -1 typically means: connection not established, SSL handshake failed, or DNS resolution failed");
            LOG_DEBUG("WiFi status check: " + String(WiFi.status() == WL_CONNECTED ? "Connected" : "Disconnected"));
        } else if (httpCode < 0) {
            errorMsg += " (HTTP client error)";
        } else if (httpCode >= 400 && httpCode < 500) {
            errorMsg += " (Client error - check authentication/authorization)";
            // Try to get error response body for more details
            String errorBody = http.getString();
            if (errorBody.length() > 0) {
                LOG_DEBUG("Error response body: " + errorBody.substring(0, min(200, (int)errorBody.length())));
            }
        } else if (httpCode >= 500) {
            errorMsg += " (Server error)";
        }
        LOG_WARNING(errorMsg);
        return 0.0;
    }

  String payload = http.getString();
  LOG_VERBOSE("Currency API payload length: " + String(payload.length()));

  StaticJsonDocument<Buffer::JSON_CURRENCY_SIZE> doc;
  DeserializationError error = deserializeJson(doc, payload);
  if (error) {
    LOG_ERROR("deserializeJson() failed: " + String(error.c_str()));
    return 0.0;
  }

  return doc[F("state")]; // Extract currency value from JSON
}

float CurrencyManager::handleCryptoResponse(HTTPClient &http) {
    LOG_VERBOSE("Calling http.GET()...");
    unsigned long startTime = millis();
    int httpCode = http.GET();
    unsigned long elapsed = millis() - startTime;
    
    LOG_DEBUG("http.GET() completed in " + String(elapsed) + "ms, code: " + String(httpCode));
    
    if (httpCode != HTTP_CODE_OK) {
        // Provide more detailed error information
        String errorMsg = "HTTP request failed with code: " + String(httpCode, DEC);
        if (httpCode == -1) {
            errorMsg += " (Connection failed - SSL handshake likely failed)";
            LOG_DEBUG("Error -1 typically means: connection not established, SSL handshake failed, or DNS resolution failed");
            LOG_DEBUG("WiFi status check: " + String(WiFi.status() == WL_CONNECTED ? "Connected" : "Disconnected"));
        } else if (httpCode < 0) {
            errorMsg += " (HTTP client error)";
        } else if (httpCode >= 400 && httpCode < 500) {
            errorMsg += " (Client error - check authentication/authorization)";
            // Try to get error response body for more details
            String errorBody = http.getString();
            if (errorBody.length() > 0) {
                LOG_DEBUG("Error response body: " + errorBody.substring(0, min(200, (int)errorBody.length())));
            }
        } else if (httpCode >= 500) {
            errorMsg += " (Server error)";
        }
        LOG_WARNING(errorMsg);
        return 0.0;
    }

  String payload = http.getString();
  LOG_VERBOSE("Crypto API payload length: " + String(payload.length()));

  StaticJsonDocument<Buffer::JSON_CURRENCY_SIZE> doc;
  DeserializationError error = deserializeJson(doc, payload);
  if (error) {
    LOG_ERROR("deserializeJson() failed: " + String(error.c_str()));
    return 0.0;
  }

  // CoinCap API v3 returns: {"timestamp":...,"data":["87214.89"]}
  // Extract first element from data array
  if (doc.containsKey(F("data")) && doc[F("data")].is<JsonArray>()) {
    JsonArray dataArray = doc[F("data")].as<JsonArray>();
    if (dataArray.size() > 0) {
      return dataArray[0].as<float>();
    }
  }
  
  LOG_ERROR("Failed to parse crypto price from response");
  return 0.0;
}

float CurrencyManager::readCurrency(const char *path) {
  LOG_DEBUG_F("Fetching currency data...");

    // Check WiFi connection before attempting request
    if (WiFi.status() != WL_CONNECTED) {
        LOG_ERROR_F("WiFi not connected, cannot fetch currency data");
        return 0.0;
    }

    int maxAttempts = Retry::MAX_ATTEMPTS_CURRENCY;
    LOG_DEBUG("Currency API path: " + String(path));

    int attempts = 0;
    bool success = false;
    float currencyValue = 0.0;

    while (attempts < maxAttempts && !success) {
        // Verify WiFi is still connected before each attempt
        if (WiFi.status() != WL_CONNECTED) {
            LOG_ERROR("WiFi disconnected during currency fetch (attempt " + String(attempts + 1) + ")");
            break;
        }
        
        yield(); // Allow system tasks before creating client
        
        // Create fresh client for each attempt to avoid BearSSL reuse issues
        BearSSL::WiFiClientSecure client;
        setupSecureClient(client, "currency API");
        HTTPClient http;
        http.setTimeout(Timing::HTTP_TIMEOUT_CURRENCY_MS);
        
        // Add reconnect delay for retries
        if (attempts > 0) {
            delay(500); // Small delay before retry
            yield();
        }

        LOG_DEBUG("Attempting HTTP connection (attempt " + String(attempts + 1) + "/" + String(maxAttempts) + ")...");
        bool connectionStarted = http.begin(client, path);
        
        if (connectionStarted) {
            String authHeader = F("Bearer ") + String(bearerTokenCurrency);
            http.addHeader(F("Authorization"), authHeader);
            http.addHeader(F("Content-Type"), F("application/json"));
            
            // Log partial token for debugging (first 20 chars)
            String tokenPreview = String(bearerTokenCurrency);
            if (tokenPreview.length() > 20) {
                tokenPreview = tokenPreview.substring(0, 20) + "...";
            }
            LOG_VERBOSE("Authorization header: Bearer " + tokenPreview);
            LOG_DEBUG("HTTP connection started, sending GET request...");
            yield(); // Allow system tasks before sending request
            
            currencyValue = handleHttpResponse(http);
            
            http.end();
            client.stop(); // Explicitly stop the client
            yield(); // Allow cleanup to complete
            
            if (currencyValue > 0.0) {
                success = true;
                LOG_DEBUG("Currency data retrieved successfully: " + String(currencyValue, 2));
            } else {
                LOG_WARNING("Failed to retrieve currency data (attempt " + String(attempts + 1) + "/" + String(maxAttempts) + ")");
            }
        } else {
            LOG_ERROR("Failed to begin HTTP connection (attempt " + String(attempts + 1) + "/" + String(maxAttempts) + ")");
            LOG_DEBUG("WiFi status: " + String(WiFi.status()) + ", RSSI: " + String(WiFi.RSSI()));
            client.stop();
            yield();
        }

        if (!success) {
            attempts++;
            if (attempts < maxAttempts) {
                LOG_WARNING("Retrying currency request (" + String(attempts) + "/" + String(maxAttempts) + ")...");
                delay(Timing::RETRY_DELAY_MS);
                yield(); // Allow system tasks
            } else {
                LOG_ERROR("Failed to get currency data after " + String(maxAttempts) + " attempts");
            }
        }
    }

    return currencyValue;
}

float CurrencyManager::readCrypto(const char *path) {
  LOG_DEBUG_F("Fetching crypto data...");

    // Check WiFi connection before attempting request
    if (WiFi.status() != WL_CONNECTED) {
        LOG_ERROR_F("WiFi not connected, cannot fetch crypto data");
        return 0.0;
    }

    int maxAttempts = Retry::MAX_ATTEMPTS_CURRENCY;
    LOG_DEBUG("Crypto API path: " + String(path));

    int attempts = 0;
    bool success = false;
    float cryptoValue = 0.0;

    while (attempts < maxAttempts && !success) {
        // Verify WiFi is still connected before each attempt
        if (WiFi.status() != WL_CONNECTED) {
            LOG_ERROR("WiFi disconnected during crypto fetch (attempt " + String(attempts + 1) + ")");
            break;
        }
        
        yield(); // Allow system tasks before creating client
        
        // Create fresh client for each attempt to avoid BearSSL reuse issues
        BearSSL::WiFiClientSecure client;
        setupSecureClient(client, "crypto API");
        HTTPClient http;
        http.setTimeout(Timing::HTTP_TIMEOUT_CURRENCY_MS);
        
        // Add reconnect delay for retries
        if (attempts > 0) {
            delay(500); // Small delay before retry
            yield();
        }

        LOG_DEBUG("Attempting HTTP connection (attempt " + String(attempts + 1) + "/" + String(maxAttempts) + ")...");
        bool connectionStarted = http.begin(client, path);
        
        if (connectionStarted) {
            String authHeader = F("Bearer ") + String(bearerTokenCrypto);
            http.addHeader(F("Authorization"), authHeader);
            http.addHeader(F("Content-Type"), F("application/json"));
            
            // Log partial token for debugging (first 20 chars)
            String tokenPreview = String(bearerTokenCrypto);
            if (tokenPreview.length() > 20) {
                tokenPreview = tokenPreview.substring(0, 20) + "...";
            }
            LOG_VERBOSE("Authorization header: Bearer " + tokenPreview);
            LOG_DEBUG("HTTP connection started, sending GET request...");
            yield(); // Allow system tasks before sending request
            
            cryptoValue = handleCryptoResponse(http);
            
            http.end();
            client.stop(); // Explicitly stop the client
            yield(); // Allow cleanup to complete
            
            if (cryptoValue > 0.0) {
                success = true;
                LOG_DEBUG("Crypto data retrieved successfully: " + String(cryptoValue, 2));
            } else {
                LOG_WARNING("Failed to retrieve crypto data (attempt " + String(attempts + 1) + "/" + String(maxAttempts) + ")");
            }
        } else {
            LOG_ERROR("Failed to begin HTTP connection (attempt " + String(attempts + 1) + "/" + String(maxAttempts) + ")");
            LOG_DEBUG("WiFi status: " + String(WiFi.status()) + ", RSSI: " + String(WiFi.RSSI()));
            client.stop();
            yield();
        }

        if (!success) {
            attempts++;
            if (attempts < maxAttempts) {
                LOG_WARNING("Retrying crypto request (" + String(attempts) + "/" + String(maxAttempts) + ")...");
                delay(Timing::RETRY_DELAY_MS);
                yield(); // Allow system tasks
            } else {
                LOG_ERROR("Failed to get crypto data after " + String(maxAttempts) + " attempts");
            }
        }
    }

    return cryptoValue;
}
