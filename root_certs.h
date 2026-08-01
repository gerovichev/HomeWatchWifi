#pragma once

#include <Arduino.h>

// Shared root CA certificates pinned for BearSSL certificate validation.
// Defined once in root_certs.cpp; each TLS client builds its own
// BearSSL::X509List from these PROGMEM buffers.

// Let's Encrypt's long-lived self-signed root (valid until 2035). Currently
// the trust anchor for gerovich.duckdns.org (currency/crypto API),
// gerovichomv.duckdns.org (OTA), and api.timezonedb.com.
extern const char ISRG_ROOT_X1[] PROGMEM;
