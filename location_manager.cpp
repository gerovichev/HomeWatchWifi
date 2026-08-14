#include "location_manager.h"
#include "constants.h"
#include "logger.h"
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#include <LittleFS.h>
#include <WiFiClientSecureBearSSL.h>
#include <WifiLocation.h>

// Root CA for api.ipify.org. Its certificate is issued by Google Trust
// Services and currently chains up to this self-signed root (valid until
// 2036), so pinning it here survives normal leaf/intermediate renewal.
static const char IPIFY_ROOT_CA[] PROGMEM = R"CERT(
-----BEGIN CERTIFICATE-----
MIICCTCCAY6gAwIBAgINAgPlwGjvYxqccpBQUjAKBggqhkjOPQQDAzBHMQswCQYD
VQQGEwJVUzEiMCAGA1UEChMZR29vZ2xlIFRydXN0IFNlcnZpY2VzIExMQzEUMBIG
A1UEAxMLR1RTIFJvb3QgUjQwHhcNMTYwNjIyMDAwMDAwWhcNMzYwNjIyMDAwMDAw
WjBHMQswCQYDVQQGEwJVUzEiMCAGA1UEChMZR29vZ2xlIFRydXN0IFNlcnZpY2Vz
IExMQzEUMBIGA1UEAxMLR1RTIFJvb3QgUjQwdjAQBgcqhkjOPQIBBgUrgQQAIgNi
AATzdHOnaItgrkO4NcWBMHtLSZ37wWHO5t5GvWvVYRg1rkDdc/eJkTBa6zzuhXyi
QHY7qca4R9gq55KRanPpsXI5nymfopjTX15YhmUPoYRlBtHci8nHc8iMai/lxKvR
HYqjQjBAMA4GA1UdDwEB/wQEAwIBhjAPBgNVHRMBAf8EBTADAQH/MB0GA1UdDgQW
BBSATNbrdP9JNqPV2Py1PsVq8JQdjDAKBggqhkjOPQQDAwNpADBmAjEA6ED/g94D
9J+uHXqnLrmvT/aDHQ4thQEd0dlq7A/Cr8deVl5c1RxYIigL9zC2L7F8AjEA8GE8
p/SgguMh1YQdc4acLa/KNJvxn7kjNuK8YAOdgLOaVsjh4rsUecrNIdSUtUlD
-----END CERTIFICATE-----
)CERT";

// Define global variables
String ip;
float latitude = 31.66;
float longitude = 34.56;
Config config;
int maxAttemptsLoc = Retry::MAX_ATTEMPTS_LOCATION;

// Path for configuration file
const char *filenamecnf = "/config.txt";

// Loads the configuration from a file
void loadConfiguration() {
  int result = LittleFS.begin();
  LOG_DEBUG_FMT("LittleFS opened with result: %d", result);

  File file = LittleFS.open(filenamecnf, "r");
  if (file) {
    StaticJsonDocument<Buffer::JSON_LOCATION_SIZE>
        doc; // Location config is small: lat, lon, ip
    DeserializationError error = deserializeJson(doc, file);

    if (error) {
      LOG_WARNING_F("Failed to read location config file, using defaults");
    } else {
      config.latitude = doc["latitude"];
      config.longitude = doc["longitude"];
      config.ip = String(doc["ip"]);
      LOG_INFO_FMT("Loaded location config: lat=%.6f, lon=%.6f, ip=%s",
                   config.latitude, config.longitude, config.ip.c_str());
    }

    file.close();
  } else {
    LOG_WARNING_F("Location config file not found");
  }

  LittleFS.end();
}

// Saves the configuration to a file
void saveConfiguration() {
  int result = LittleFS.begin();
  LOG_DEBUG_FMT("LittleFS opened for writing: %d", result);

  File file = LittleFS.open(filenamecnf, "w");
  if (!file) {
    LOG_ERROR_F("Failed to create location config file");
    LittleFS.end();
    return;
  }

  StaticJsonDocument<Buffer::JSON_LOCATION_SIZE>
      doc; // Location config is small: lat, lon, ip
  doc["latitude"] = config.latitude;
  doc["longitude"] = config.longitude;
  doc["ip"] = config.ip;

  if (serializeJson(doc, file) == 0) {
    LOG_ERROR_F("Failed to write location config to file");
  } else {
    LOG_INFO_FMT("Location config saved: lat=%.6f, lon=%.6f, ip=%s",
                 config.latitude, config.longitude, config.ip.c_str());
  }

  file.close();
  LittleFS.end();
}

// Sets time via NTP for x.509 validation
void setClock() {
  LOG_DEBUG_F("Setting system clock for SSL validation...");
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");

  time_t now = time(nullptr);
  int waitCount = 0;
  // Wait for time to be set, but with a timeout (approx 25 seconds)
  while (now < 8 * 3600 * 2 && waitCount < 50) {
    delay(Timing::NTP_SYNC_WAIT_MS);
    if (++waitCount % 10 == 0) {
      LOG_VERBOSE_F("Waiting for NTP time sync...");
    }
    now = time(nullptr);
  }

  if (waitCount >= 50) {
    LOG_WARNING_F("NTP sync timeout, continuing with unsynchronized time");
  }

  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  const char *timeStr = asctime(&timeinfo);
  LOG_DEBUG_FMT("System clock set: %s", timeStr ? timeStr : "n/a");
}

// Get location via Google API using WiFi data
void getLocationAPI(const String &ip) {
  LOG_INFO_F("Fetching location via Google Geolocation API...");

  setClock();

  // This does a blocking WiFi scan and then a TLS handshake inside the library,
  // so it is the longest single stall in the cycle — but it only runs when the
  // external IP changes. 160 MHz is what keeps it under the watchdog.
  WifiLocation location(googleApiKey);
  location_t loc = location.getGeoFromWiFi();
  String status = location.wlStatusStr(location.getStatus());

  if (!status.equals("OK")) {
    LOG_ERROR_FMT("Google Geolocation API returned status: %s", status.c_str());
    return;
  }

  latitude = loc.lat;
  longitude = loc.lon;

  LOG_INFO_FMT("Location updated: lat=%.7f, lon=%.7f", latitude, longitude);
  LOG_DEBUG_FMT("Location accuracy: %u meters", loc.accuracy);
  // Guarded explicitly: LOG_*_FMT arguments are evaluated at the call site even
  // when the level is disabled, and getSurroundingWiFiJson() is not a getter —
  // it runs a second blocking WiFi.scanNetworks() and builds a multi-KB String.
  // Unguarded, that cost was paid on every location fetch at LOG_LEVEL_NONE.
  if (Logger::getInstance().getLogLevel() >= LOG_LEVEL_VERBOSE) {
    LOG_VERBOSE_FMT("WiFi scan data: %s",
                    location.getSurroundingWiFiJson().c_str());
  }

  config.latitude = latitude;
  config.longitude = longitude;
  config.ip = ip;
}

// Get external IP address using an API
String getIp() {
  LOG_INFO_F("Fetching external IP address...");

  // BearSSL needs a real system clock to validate the certificate's
  // notBefore/notAfter dates; this is a no-op once NTP has already synced.
  setClock();

  String payload;
  // rootCA is declared before client so that client is destroyed first — the
  // client must never outlive the trust anchors it points at.
  BearSSL::X509List rootCA(IPIFY_ROOT_CA);
  BearSSL::WiFiClientSecure client;
  client.setTrustAnchors(&rootCA);
  client.setBufferSizes(Buffer::TLS_RX_SIZE, Buffer::TLS_TX_SIZE);
  HTTPClient http;

  String path = "https://api.ipify.org";
  int attempts = 0;
  bool success = false;

  while (attempts < maxAttemptsLoc && !success) {
    if (http.begin(client, path)) {
      LOG_DEBUG_FMT("IP retrieval attempt %d/%d", attempts + 1, maxAttemptsLoc);
      // No session cache here: this runs hourly, far enough apart that the
      // ticket has usually expired, and it is a single request per pass.
      int httpCode = http.GET(); // Send the request

      if (httpCode == HTTP_CODE_OK) {
        payload = http.getString(); // Get the response payload
        LOG_INFO_FMT("External IP retrieved: %s", payload.c_str());
        success = true;
        // Bug fix #7: removed redundant `maxAttemptsLoc = 1` — the while
        // condition already checks `!success`.
      } else {
        LOG_WARNING_FMT("IP retrieval HTTP error: %d", httpCode);
      }

      http.end();
    } else {
      LOG_ERROR_F("Failed to begin IP retrieval HTTP connection");
    }

    if (!success) {
      attempts++;
      if (attempts < maxAttemptsLoc) {
        LOG_WARNING_FMT("Retrying IP retrieval (%d/%d)...", attempts,
                        maxAttemptsLoc);
        delay(Timing::RETRY_DELAY_MS);
      } else {
        LOG_ERROR_FMT("Failed to get IP after %d attempts.", maxAttemptsLoc);
        // Don't restart immediately - allow device to continue with cached
        // location if available Only restart if this is critical for device
        // operation
        if (config.latitude == 0 && config.longitude == 0) {
          LOG_ERROR_F("No cached location available, restarting device...");
          delay(1000);
          ESP.restart();
        } else {
          LOG_WARNING_F("Using cached location due to IP retrieval failure");
        }
      }
    }
  }

  return payload;
}

// Initialize location by loading config or calling API
void location_init() {
  LOG_INFO_F("Initializing location services...");

  // Load the cached location BEFORE getIp(): getIp() reads config.latitude to
  // decide whether a failed IP lookup is fatal, and `config` is a zero-init
  // global. With the old order that check always saw 0 on the first call of a
  // boot, so a transient ipify failure rebooted the device even though a
  // perfectly good cached location was sitting in LittleFS.
  loadConfiguration();
  ip = getIp();

  if (ip.equals(config.ip) && config.latitude != 0) {
    latitude = config.latitude;
    longitude = config.longitude;
    LOG_INFO_FMT("Using cached location: lat=%.7f, lon=%.7f", latitude,
                 longitude);
  } else {
    LOG_INFO_F("IP changed or no cached location, fetching new location...");
    getLocationAPI(ip);
    saveConfiguration();
  }
}
