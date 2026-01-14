#include "WiFiSetup.h"

#include <WiFiManager.h>  // https://github.com/tzapu/WiFiManager
#include "Secret.h"
#include "logger.h"
#include "led_display.h"
#include "constants.h"

// Check if WiFi credentials are saved in EEPROM
bool WIFISetup::hasSavedCredentials() {
    // WiFiManager stores credentials in EEPROM
    // Try to read saved SSID - this works even when not connected
    // If WiFi was never configured, SSID will be empty
    String savedSSID = WiFi.SSID();
    bool hasCredentials = (savedSSID.length() > 0);
    
    // Additional check: try to get WiFi config
    // If credentials exist but network is unavailable, SSID will still be non-empty
    if (!hasCredentials) {
        // Try reading from WiFi config directly
        struct station_config config;
        wifi_station_get_config(&config);
        hasCredentials = (strlen((char*)config.ssid) > 0);
    }
    
    LOG_DEBUG("Saved credentials check: " + String(hasCredentials ? "Found" : "Not found"));
    if (hasCredentials) {
        LOG_DEBUG("Saved SSID: " + savedSSID);
    }
    
    return hasCredentials;
}

// Attempt direct connection to saved WiFi credentials with retries
bool WIFISetup::attemptDirectConnection(int maxAttempts) {
    LOG_INFO_F("Attempting direct WiFi connection to saved credentials...");
    
    for (int attempt = 1; attempt <= maxAttempts; attempt++) {
        LOG_INFO("WiFi connection attempt " + String(attempt) + "/" + String(maxAttempts));
        
        // WiFi.begin() without parameters uses saved credentials from EEPROM
        WiFi.mode(WIFI_STA);
        WiFi.begin();  // Использует сохраненные учетные данные
        
        unsigned long startTime = millis();
        while (WiFi.status() != WL_CONNECTED) {
            if (millis() - startTime > Network::WIFI_INIT_SINGLE_ATTEMPT_TIMEOUT_MS) {
                LOG_WARNING("Connection attempt " + String(attempt) + " timed out");
                break;
            }
            delay(500);
            yield();  // Allow other tasks to run
        }
        
        if (WiFi.status() == WL_CONNECTED) {
            LOG_INFO("✓ WiFi connected successfully on attempt " + String(attempt));
            return true;
        }
        
        if (attempt < maxAttempts) {
            LOG_WARNING("Connection failed, retrying in " + 
                       String(Network::WIFI_INIT_RETRY_DELAY_MS / 1000) + " seconds...");
            delay(Network::WIFI_INIT_RETRY_DELAY_MS);
        }
    }
    
    LOG_ERROR("Failed to connect after " + String(maxAttempts) + " attempts");
    return false;
}

// Function to initialize and connect to WiFi
void WIFISetup::wifi_init() {
    LOG_INFO_F("Starting WiFi initialization...");
    LOG_DEBUG("Device MAC: " + WiFi.macAddress());

    // First, try to connect using saved credentials with multiple retries
    if (hasSavedCredentials()) {
        LOG_INFO_F("Found saved WiFi credentials, attempting direct connection...");
        
        if (attemptDirectConnection(Network::WIFI_INIT_RETRY_ATTEMPTS)) {
            // Connection successful
            WiFi.setAutoReconnect(true);
            WiFi.persistent(true);
            
            // Display connected network name on LED display
            String connectedSSID = WiFi.SSID();
            printText(connectedSSID);
            delay(2000);  // Show SSID for 2 seconds
            
            // Log detailed connection information
            LOG_INFO("✓ WiFi connected successfully!");
            LOG_INFO("  SSID: " + connectedSSID);
            LOG_INFO("  IP: " + WiFi.localIP().toString());
            LOG_INFO("  Gateway: " + WiFi.gatewayIP().toString());
            LOG_DEBUG("  Subnet: " + WiFi.subnetMask().toString());
            LOG_DEBUG("  DNS: " + WiFi.dnsIP().toString());
            LOG_DEBUG("  MAC: " + WiFi.macAddress());
            LOG_DEBUG("  RSSI: " + String(WiFi.RSSI()) + " dBm");
            LOG_DEBUG("  Channel: " + String(WiFi.channel()));
            return;
        } else {
            // Connection failed, but we have saved credentials
            // Continue trying in background, don't start AP mode
            LOG_WARNING_F("WiFi connection failed, but saved credentials exist");
            LOG_INFO_F("Device will continue trying to reconnect in background");
            LOG_INFO_F("AP mode will NOT be started - device will keep retrying saved WiFi");
            printText(F("WiFi RETRY"));
            
            // Set auto-reconnect for background reconnection
            WiFi.setAutoReconnect(true);
            WiFi.persistent(true);
            return;
        }
    }
    
    // No saved credentials - use WiFiManager for initial setup
    LOG_INFO_F("No saved WiFi credentials found, starting configuration portal...");
    WiFiManager wifiManager;

    // Set connect timeout to 180 seconds
    wifiManager.setConnectTimeout(180);

    // Set dark theme for WiFiManager web interface
    wifiManager.setClass(F("invert"));

    LOG_DEBUG("Config portal SSID: " + String(wifi_name));

    // Start the AP mode for initial configuration
    LOG_INFO_F("Starting WiFi configuration portal (AP mode)...");
    bool res = wifiManager.autoConnect(wifi_name, wifi_pass);

    if (!res) {
        LOG_ERROR_F("Failed to connect to WiFi or configuration timeout reached");
        LOG_WARNING_F("Device will continue with limited functionality");
        printText(F("WiFi FAIL"));
    } else {
        // Stop the configuration web portal
        wifiManager.stopWebPortal();

        // Set WiFi auto-reconnect and persistence
        WiFi.setAutoReconnect(true);
        WiFi.persistent(true);

        // Display connected network name on LED display
        String connectedSSID = WiFi.SSID();
        printText(connectedSSID);
        delay(2000);  // Show SSID for 2 seconds

        // Log detailed connection information
        LOG_INFO("✓ WiFi connected successfully!");
        LOG_INFO("  SSID: " + connectedSSID);
        LOG_INFO("  IP: " + WiFi.localIP().toString());
        LOG_INFO("  Gateway: " + WiFi.gatewayIP().toString());
        LOG_DEBUG("  Subnet: " + WiFi.subnetMask().toString());
        LOG_DEBUG("  DNS: " + WiFi.dnsIP().toString());
        LOG_DEBUG("  MAC: " + WiFi.macAddress());
        LOG_DEBUG("  RSSI: " + String(WiFi.RSSI()) + " dBm");
        LOG_DEBUG("  Channel: " + String(WiFi.channel()));
    }
}

// Function to reset saved WiFi credentials
void WIFISetup::wifi_reset() {
    LOG_WARNING_F("Resetting WiFi credentials...");
    LOG_WARNING("Current SSID: " + WiFi.SSID() + " will be forgotten");
    
    WiFiManager wifiManager;

    // Reset saved WiFi credentials
    wifiManager.resetSettings();
    LOG_INFO_F("WiFi credentials reset successfully");
    LOG_INFO_F("Device will need to be reconfigured on next boot");
}

// Check if WiFi is connected
bool WIFISetup::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

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
        return true;  // Already connected
    }
    
    if (!hasSavedCredentials()) {
        LOG_DEBUG_F("No saved credentials for reconnection");
        return false;
    }
    
    LOG_INFO_F("Attempting to reconnect to saved WiFi...");
    WiFi.mode(WIFI_STA);
    WiFi.begin();  // Uses saved credentials
    
    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED) {
        if (millis() - startTime > Network::WIFI_CONNECT_TIMEOUT_MS) {
            LOG_DEBUG_F("Reconnection timeout");
            return false;
        }
        delay(500);
        yield();
    }
    
    LOG_INFO("WiFi reconnected! IP: " + WiFi.localIP().toString());
    return true;
}
