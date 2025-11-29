#pragma once

// Timing constants
namespace Timing {
    constexpr int CLOCK_INTERVAL_SEC = 5;
    constexpr int DATA_UPDATE_INTERVAL_SEC = 900;  // 15 minutes
    constexpr int WIFI_TIMEOUT_MS = 10000;
    constexpr int HTTP_TIMEOUT_MS = 1500;
    constexpr int HTTP_TIMEOUT_CURRENCY_MS = 1500;
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
}

// Display constants
namespace Display {
    constexpr int SCROLL_SPEED_MS = 50;
    constexpr int PAUSE_TIME_MS = 1000;
    constexpr int DISPLAY_CYCLE_LENGTH = 21;
    constexpr int INTENSITY_DAY = 2;
    constexpr int INTENSITY_NIGHT = 0;
}

// Buffer sizes
namespace Buffer {
    constexpr size_t LED_BUFFER_SIZE = 512;
    constexpr size_t JSON_WEATHER_SIZE = 1024;
    constexpr size_t JSON_TIMEZONE_SIZE = 512;
    constexpr size_t JSON_CURRENCY_SIZE = 512;
    constexpr size_t JSON_LOCATION_SIZE = 256;
    constexpr size_t PATH_BUFFER_SIZE = 512;
    constexpr size_t TIME_STRING_SIZE = 20;
    constexpr size_t TEMP_STRING_SIZE = 8;
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
}


