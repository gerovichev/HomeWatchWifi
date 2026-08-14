#pragma once

// Timing constants
namespace Timing {
    constexpr int CLOCK_INTERVAL_SEC = 6;
    constexpr int DATA_UPDATE_INTERVAL_SEC = 1200;  // 20 minutes
    constexpr int WIFI_TIMEOUT_MS = 10000;
    constexpr int HTTP_TIMEOUT_MS = 1500;
    constexpr int HTTP_TIMEOUT_CURRENCY_MS = 3000;
    constexpr int OTA_CLIENT_TIMEOUT_MS = 2000;
    constexpr int RETRY_DELAY_MS = 2000;
    constexpr int MQTT_RECONNECT_DELAY_MS = 5000;
    constexpr int NTP_SYNC_WAIT_MS = 500;
    constexpr int SERIAL_INIT_DELAY_MS = 500;
}

// Retry constants
namespace Retry {
    constexpr int MAX_ATTEMPTS_WEATHER = 3;
    constexpr int MAX_ATTEMPTS_LOCATION = 3;
    constexpr int MAX_ATTEMPTS_TIMEZONE = 3;
    constexpr int MAX_ATTEMPTS_MQTT = 3;
    constexpr int MAX_ATTEMPTS_CURRENCY = 3;
}

// Display constants
namespace Display {
    constexpr int SCROLL_SPEED_MS = 50;
    constexpr int PAUSE_TIME_MS = 1000;
    // Bug fix #2: use this constant to size displaySequence[] in clock.h so
    // adding new items cannot silently overflow the array.
    constexpr int MAX_DISPLAY_SEQUENCE = 28; // Base 22 + DHT 4 + spare 2
    constexpr int DISPLAY_CYCLE_LENGTH = 21; // Kept for reference; prefer MAX_DISPLAY_SEQUENCE
    constexpr int INTENSITY_DAY = 2;
    constexpr int INTENSITY_NIGHT = 0;
}

// Buffer sizes
namespace Buffer {
    constexpr size_t LED_BUFFER_SIZE = 128;
    constexpr size_t JSON_WEATHER_SIZE = 1024;
    constexpr size_t JSON_TIMEZONE_SIZE = 512;
    constexpr size_t JSON_CURRENCY_SIZE = 512;
    constexpr size_t JSON_LOCATION_SIZE = 256;
    constexpr size_t PATH_BUFFER_SIZE = 512;
    constexpr size_t TIME_STRING_SIZE = 20;
    constexpr size_t TEMP_STRING_SIZE = 8;

    // BearSSL TLS record buffers.
    //
    // The default is 16 KB receive, which alone is over half the free heap.
    // _connectSSL() takes that buffer, then a ~3.4 KB br_x509_minimal_context,
    // then the parsed trust anchor — ~21 KB in one go, which is what threw
    // "Unhandled C++ exception: OOM" from operator new.
    //
    // 4 KB holds every response this firmware fetches (ipify ~15 B, currency
    // ~200 B, weather ~1 KB) with room for a normal 2-3 certificate chain in
    // the handshake. If a server ever sends a TLS record larger than this the
    // connection simply fails and is retried — it does not crash — so if
    // handshakes start failing against one host, raise this to 8192 first.
    constexpr int TLS_RX_SIZE = 4096;
    constexpr int TLS_TX_SIZE = 512;
}

// Sensor constants
namespace Sensor {
    constexpr int DHT_PIN = 12;
    constexpr float HUMIDITY_DELTA_DEFAULT = 0.00;
}

// Network constants
namespace Network {
    constexpr int WIFI_RECONNECT_ATTEMPTS = 20;
    constexpr unsigned long WIFI_CONNECT_TIMEOUT_MS = 10000;
    constexpr int WIFI_INIT_RETRY_ATTEMPTS = 10;  // Number of connection attempts at startup
    constexpr unsigned long WIFI_INIT_RETRY_DELAY_MS = 5000;  // Delay between attempts (5 seconds)
    constexpr unsigned long WIFI_INIT_SINGLE_ATTEMPT_TIMEOUT_MS = 15000;  // Timeout for a single attempt (15 seconds)
}


