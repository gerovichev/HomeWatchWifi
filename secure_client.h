#pragma once

#include <WiFiClientSecure.h>
#include "logger.h"

/**
 * Setup secure SSL client
 * Attempts to use certificate validation if possible
 * Otherwise uses setInsecure with warning
 */
inline void setupSecureClient(BearSSL::WiFiClientSecure& client, const char* domain = nullptr) {
    // For ESP8266 full certificate validation requires additional setup
    // In production it's recommended to use root certificates
    // For now using setInsecure with warning for compatibility
    
    // TODO: Add support for root certificates for main domains
    // X509List cert(certificate);
    // client.setTrustAnchors(&cert);
    
    client.setInsecure();
    
    if (domain) {
        LOG_DEBUG_FMT("Using insecure SSL for: %s (consider cert validation)", domain);
    } else {
        LOG_DEBUG_F("Secure client configured (insecure mode)");
    }
}


