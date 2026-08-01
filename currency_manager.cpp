// CurrencyManager.cpp
#include "currency_manager.h"
#include "clock.h"
#include "constants.h"
#include "logger.h"
#include "location_manager.h"

// Root CA for gerovich.duckdns.org. Its certificate is issued by Let's
// Encrypt and currently chains up to this self-signed ISRG root (valid
// until 2035), so pinning it here survives normal leaf/intermediate renewal.
static const char DUCKDNS_ROOT_CA[] PROGMEM = R"CERT(
-----BEGIN CERTIFICATE-----
MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw
TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh
cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4
WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu
ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY
MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc
h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+
0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U
A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW
T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH
B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC
B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv
KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn
OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn
jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw
qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI
rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV
HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq
hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL
ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ
3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK
NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5
ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur
TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC
jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc
oyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq
4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA
mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d
emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=
-----END CERTIFICATE-----
)CERT";

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
        BearSSL::X509List rootCA(DUCKDNS_ROOT_CA);
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
