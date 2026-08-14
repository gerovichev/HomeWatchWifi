#include "global_config.h"
#include "constants.h"

// Define the global variables
String lang_weather;
// Bug fix #6: changed from unsigned int (16-bit, max 65535) to unsigned long
// (32-bit) so Unix epoch timestamps fit without silent truncation.
unsigned long sunrise;
unsigned long sunset;

String version_prg = "260808";
char grad = '\x60';

float humidity_delta = 0.00;
String hostname_m;

boolean isOTAreq = true;
boolean isMQTT = false;
String nameofWatch;

String macAddrSt;

boolean IS_DHT_CONNECTED = false;
bool isWebClientNeeded = true;
boolean isReadWeather = true;

// Function to return the degree character based on the language
char getGradValue() {
  return grad;
}

// Initialize the device configuration based on the MAC address
void initPerDevice() {
  String macAddr = WiFi.macAddress();
  LOG_INFO_FMT("MAC: %s", macAddr.c_str());

  macAddrSt = macAddr;

  // Look up device config in the flat array (replaces std::map)
  const DeviceConfig* config = findDeviceConfig(macAddr);

  if (config != nullptr) {
    lang_weather = config->lang_weather;
    hostname_m = config->hostname_m;
    IS_DHT_CONNECTED = config->IS_DHT_CONNECTED;
    isWebClientNeeded = config->isWebClientNeeded;
    isReadWeather = config->isReadWeather;
    humidity_delta = config->humidity_delta;
    nameofWatch = config->nameofWatch;
    isOTAreq = config->isOTAreq;
    isMQTT = config->isMQTT;
    setIntensity(config->intensity);  // Set LED intensity based on the config
    mqtt_topic_str = hostname_m + mqtt_topic;   // const char* appended directly

    LOG_INFO_FMT("Device configured: %s", hostname_m.c_str());
    LOG_DEBUG_FMT("Language: %s", lang_weather.c_str());
    LOG_DEBUG_FMT("DHT22: %s", IS_DHT_CONNECTED ? "connected" : "disconnected");
    LOG_DEBUG_FMT("MQTT: %s", isMQTT ? "enabled" : "disabled");

  } else {
    // Set default values if MAC address is not found in the config array
    lang_weather = "en";
    hostname_m = "ESP_Unknown";
    IS_DHT_CONNECTED = false;
    isWebClientNeeded = true;
    isReadWeather = true;
    nameofWatch = "New";
    
    LOG_WARNING_F("MAC address not found in config, using defaults");
  }

  LOG_INFO_FMT("Hostname: %s", hostname_m.c_str());

  // Days of week are now accessed via getDayOfWeek() using PROGMEM
}

// Accessor for days of week from PROGMEM
const char* const daysRu[] PROGMEM = {"Вс.", "Пн.", "Вт.", "Ср.", "Чт.", "Пт.", "Сб."};
const char* const daysEn[] PROGMEM = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

const char* getDayOfWeek(int day) {
  if (day < 0 || day > 6) return "";
  if (!lang_weather.compareTo("ru")) {
    return daysRu[day];
  } else {
    return daysEn[day];
  }
}

// Function to verify Wi-Fi connection
// Bug fix #5: the original loop called WiFi.reconnect() with no delay, no
// timeout, and no watchdog feeding — if WiFi never recovered the device would
// hang forever. Now limited to Timing::WIFI_TIMEOUT_MS with yield() safety.
void verifyWifi() {
  if (WiFi.status() == WL_CONNECTED && WiFi.localIP() != IPAddress(0, 0, 0, 0)) {
    return; // Already connected — fast path
  }

  WiFi.reconnect();
  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED || WiFi.localIP() == IPAddress(0, 0, 0, 0)) {
    if (millis() - startTime > Timing::WIFI_TIMEOUT_MS) {
      LOG_WARNING_F("verifyWifi() timed out, continuing with degraded connectivity");
      return;
    }
    yield();
    delay(200);
  }
}

// Function to get a two-digit number as a string (with leading zero if necessary)
String getNumberWithZerro(int dig) {
    return (dig < 10) ? "0" + String(dig, DEC) : String(dig, DEC);
}

// Wrapper function for drawing text on the display
void drawString(const String& tape) {
  drawStringMax(tape);
}
