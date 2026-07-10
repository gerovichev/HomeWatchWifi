#include "WiFiSetup.h"

#include "Secret.h"
#include "constants.h"
#include "led_display.h"
#include "logger.h"
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager

// Helper to log WiFi connection details — avoids duplicating 8 log lines
static void logConnectionInfo() {
  String connectedSSID = WiFi.SSID();
  printText(connectedSSID);
  delay(2000); // Show SSID for 2 seconds

  LOG_INFO_F("WiFi connected successfully!");
  LOG_INFO_FMT("  SSID: %s", connectedSSID.c_str());
  LOG_INFO_FMT("  IP: %s", WiFi.localIP().toString().c_str());
  LOG_INFO_FMT("  Gateway: %s", WiFi.gatewayIP().toString().c_str());
  LOG_DEBUG_FMT("  Subnet: %s", WiFi.subnetMask().toString().c_str());
  LOG_DEBUG_FMT("  DNS: %s", WiFi.dnsIP().toString().c_str());
  LOG_DEBUG_FMT("  MAC: %s", WiFi.macAddress().c_str());
  LOG_DEBUG_FMT("  RSSI: %d dBm", WiFi.RSSI());
  LOG_DEBUG_FMT("  Channel: %d", WiFi.channel());
}

// Check if WiFi credentials are saved in EEPROM
bool WIFISetup::hasSavedCredentials() {
  // WiFiManager stores credentials in EEPROM
  // Try to read saved SSID - this works even when not connected
  // If WiFi was never configured, SSID will be empty
  String savedSSID = WiFi.SSID();
  bool hasCredentials = (savedSSID.length() > 0);

  // Additional check: try to get WiFi config
  // If credentials exist but network is unavailable, SSID will still be
  // non-empty
  if (!hasCredentials) {
    // Try reading from WiFi config directly
    struct station_config config;
    wifi_station_get_config(&config);
    hasCredentials = (strlen((char *)config.ssid) > 0);
  }

  LOG_DEBUG_FMT("Saved credentials check: %s",
                hasCredentials ? "Found" : "Not found");
  if (hasCredentials) {
    LOG_DEBUG_FMT("Saved SSID: %s", savedSSID.c_str());
  }

  return hasCredentials;
}

// Attempt direct connection to saved WiFi credentials with retries
bool WIFISetup::attemptDirectConnection(int maxAttempts) {
  LOG_INFO_F("Attempting direct WiFi connection to saved credentials...");

  // Avoid writing credentials to flash on every reconnect attempt.
  WiFi.persistent(false);

  for (int attempt = 1; attempt <= maxAttempts; attempt++) {
    LOG_INFO_FMT("WiFi connection attempt %d/%d", attempt, maxAttempts);

    // Read credentials explicitly since WiFi.begin() without args is unreliable
    String savedSSID = WiFi.SSID();
    String savedPass = WiFi.psk();

    WiFi.mode(WIFI_STA);
    if (savedSSID.length() > 0) {
      WiFi.begin(savedSSID.c_str(), savedPass.c_str());
    } else {
      WiFi.begin();
    }

    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED) {
      if (millis() - startTime > Network::WIFI_INIT_SINGLE_ATTEMPT_TIMEOUT_MS) {
        LOG_WARNING_FMT("Connection attempt %d timed out", attempt);
        break;
      }
      delay(500);
      yield(); // Allow other tasks to run
    }

    if (WiFi.status() == WL_CONNECTED) {
      LOG_INFO_FMT("WiFi connected successfully on attempt %d", attempt);
      return true;
    }

    if (attempt < maxAttempts) {
      LOG_WARNING_FMT("Connection failed, retrying in %lu seconds...",
                      Network::WIFI_INIT_RETRY_DELAY_MS / 1000UL);
      delay(Network::WIFI_INIT_RETRY_DELAY_MS);
    }
  }

  LOG_ERROR_FMT("Failed to connect after %d attempts", maxAttempts);
  return false;
}

// Function to initialize and connect to WiFi
void WIFISetup::wifi_init() {
  LOG_INFO_F("Starting WiFi initialization...");
  LOG_DEBUG_FMT("Device MAC: %s", WiFi.macAddress().c_str());

  // First, try to connect using saved credentials with multiple retries
  if (hasSavedCredentials()) {
    LOG_INFO_F("Found saved WiFi credentials, attempting direct connection...");

    if (attemptDirectConnection(Network::WIFI_INIT_RETRY_ATTEMPTS)) {
      // Connection successful
      WiFi.setAutoReconnect(true);
      WiFi.persistent(false);
      logConnectionInfo();
      return;
    } else {
      // Connection failed with saved credentials, start AP mode
      LOG_WARNING_F(
          "WiFi connection failed with saved credentials after max attempts");
      LOG_INFO_F("Starting AP mode for reconfiguration...");
      printText(F("WIFI AP"));
    }
  } else {
    // No saved credentials - use WiFiManager for initial setup
    LOG_INFO_F(
        "No saved WiFi credentials found, starting configuration portal...");
  }

  WiFiManager wifiManager;

  // Set connect timeout to 15 seconds (reduced from 180 to prevent getting stuck)
  wifiManager.setConnectTimeout(15);

  // Set timeout for the configuration portal so it doesn't block forever
  wifiManager.setConfigPortalTimeout(180);

  // Callback to reboot upon save to prevent hanging when switching modes
  wifiManager.setSaveConfigCallback([]() {
    LOG_INFO_F("WiFi credentials saved. Restarting to apply...");
    delay(1000);
    ESP.restart();
  });

  // Set dark theme for WiFiManager web interface
  wifiManager.setClass(F("invert"));

  LOG_DEBUG_FMT("Config portal SSID: %s", wifi_name);

  // Start the AP mode for initial configuration directly
  LOG_INFO_F("Starting WiFi configuration portal (AP mode)...");
  bool res = wifiManager.startConfigPortal(wifi_name, wifi_pass);

  if (!res) {
    LOG_ERROR_F("Failed to connect to WiFi or configuration timeout reached");
    LOG_WARNING_F("Device will restart to retry connecting to saved networks.");
    printText(F("WiFi FAIL"));
    delay(3000);
    ESP.restart();
  } else {
    // Stop the configuration web portal
    wifiManager.stopWebPortal();

    // Set WiFi auto-reconnect and persistence
    WiFi.setAutoReconnect(true);
    WiFi.persistent(false);
    logConnectionInfo();
  }
}

// Function to reset saved WiFi credentials
void WIFISetup::wifi_reset() {
  LOG_WARNING_F("Resetting WiFi credentials...");
  LOG_WARNING_FMT("Current SSID: %s will be forgotten", WiFi.SSID().c_str());

  WiFiManager wifiManager;

  // Reset saved WiFi credentials
  wifiManager.resetSettings();
  LOG_INFO_F("WiFi credentials reset successfully");
  LOG_INFO_F("Device will need to be reconfigured on next boot");
}

// Check if WiFi is connected
bool WIFISetup::isConnected() { return WiFi.status() == WL_CONNECTED; }

// Get WiFi status as a readable string
String WIFISetup::getStatusString() {
  switch (WiFi.status()) {
  case WL_CONNECTED:
    return "Connected";
  case WL_NO_SSID_AVAIL:
    return "SSID not available";
  case WL_CONNECT_FAILED:
    return "Connection failed";
  case WL_IDLE_STATUS:
    return "Idle";
  case WL_DISCONNECTED:
    return "Disconnected";
  default:
    return "Unknown (" + String(WiFi.status()) + ")";
  }
}

// Attempt to reconnect to saved WiFi
bool WIFISetup::attemptReconnect() {
  if (WiFi.status() == WL_CONNECTED) {
    return true; // Already connected
  }

  if (!hasSavedCredentials()) {
    LOG_DEBUG_F("No saved credentials for reconnection");
    return false;
  }

  LOG_INFO_F("Attempting to reconnect to saved WiFi...");
  String savedSSID = WiFi.SSID();
  String savedPass = WiFi.psk();

  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  if (savedSSID.length() > 0) {
    WiFi.begin(savedSSID.c_str(), savedPass.c_str());
  } else {
    WiFi.begin();
  }

  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED) {
    if (millis() - startTime > Network::WIFI_CONNECT_TIMEOUT_MS) {
      LOG_DEBUG_F("Reconnection timeout");
      return false;
    }
    delay(500);
    yield();
  }

  LOG_INFO_FMT("WiFi reconnected! IP: %s", WiFi.localIP().toString().c_str());
  return true;
}
