#include "WiFiSetup.h"

#include <WiFiManager.h>  // https://github.com/tzapu/WiFiManager
#include "Secret.h"
#include "logger.h"
#include "led_display.h"

// Function to initialize and connect to WiFi using WiFiManager
void WIFISetup::wifi_init() {
    LOG_INFO_F("Starting WiFi initialization...");
    LOG_DEBUG("Device MAC: " + WiFi.macAddress());

    WiFiManager wifiManager;

    // Optional: Set connect timeout to 180 seconds
    wifiManager.setConnectTimeout(180);

    // Set dark theme for WiFiManager web interface
    wifiManager.setClass(F("invert"));

    LOG_DEBUG("Config portal SSID: " + String(wifi_name));

    // Try to auto-connect using saved credentials or start the AP mode
    LOG_INFO_F("Attempting WiFi connection (saved credentials or AP mode)...");
    bool res = wifiManager.autoConnect(wifi_name, wifi_pass);

    if (!res) {
        LOG_ERROR_F("Failed to connect to WiFi or timeout reached");
        LOG_WARNING_F("Device will restart or continue with limited functionality");
        printText(F("WiFi FAIL"));
        // Optionally restart the ESP device if connection fails
        // ESP.restart();
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
