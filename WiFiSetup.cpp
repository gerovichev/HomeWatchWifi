#include "WiFiSetup.h"

#include <WiFiManager.h>  // https://github.com/tzapu/WiFiManager
#include "Secret.h"

// Function to initialize and connect to WiFi using WiFiManager
void WIFISetup::wifi_init() {
    if (Serial) Serial.println(F("Start load WIFI"));

    WiFiManager wifiManager;

    // Optional: Set connect timeout to 180 seconds
    wifiManager.setConnectTimeout(180);

    // Set dark theme for WiFiManager web interface
    wifiManager.setClass(F("invert"));

    // Try to auto-connect using saved credentials or start the AP mode
    bool res = wifiManager.autoConnect(wifi_name, wifi_pass);

    if (!res) {
        if (Serial) Serial.println(F("Failed to connect or hit timeout"));
        // Optionally restart the ESP device if connection fails
        // ESP.restart();
    } else {
        // Stop the configuration web portal
        wifiManager.stopWebPortal();

        // Set WiFi auto-reconnect and persistence
        WiFi.setAutoReconnect(true);
        WiFi.persistent(true);

        if (Serial) {
            Serial.println(F("connected...yeey :)"));
            Serial.println(F("Ready"));
            Serial.print(F("IP address: "));
            Serial.println(WiFi.localIP());
            Serial.println(F("WIFI handled"));
        }
    }
}

// Function to reset saved WiFi credentials
void WIFISetup::wifi_reset() {
    WiFiManager wifiManager;

    // Reset saved WiFi credentials
    wifiManager.resetSettings();
}
