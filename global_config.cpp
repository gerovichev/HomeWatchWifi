#include "global_config.h"

// Define the global variables
String lang_weather;
unsigned int sunrise;
unsigned int sunset;

String version_prg = "251207";
char grad = '\x60';

float humidity_delta = 0.00;
String hostname_m;

boolean isOTAreq = true;
boolean isMQTT = false;
String nameofWatch;

String macAddrSt;
String daysOfTheWeek[7];
boolean IS_DHT_CONNECTED = false;
bool isWebClientNeeded = true;
boolean isReadWeather = true;

// Function to return the degree character based on the language
char getGradValue() {
  return grad;
}

// Initialize the device configuration based on the MAC address
void initPerDevice() {
  setDeviceConfig();  // Load configuration for the device

  String macAddr = WiFi.macAddress();
  LOG_INFO("MAC: " + macAddr);

  macAddrSt = macAddr;

  // If the device MAC is found in the configuration map, apply the settings
  if (configMap.find(macAddr) != configMap.end()) {
    DeviceConfig& config = configMap[macAddr];

    lang_weather = config.lang_weather;
    hostname_m = config.hostname_m;
    IS_DHT_CONNECTED = config.IS_DHT_CONNECTED;
    isWebClientNeeded = config.isWebClientNeeded;
    isReadWeather = config.isReadWeather;
    humidity_delta = config.humidity_delta;
    nameofWatch = config.nameofWatch;
    isOTAreq = config.isOTAreq;
    isMQTT = config.isMQTT;
    setIntensity(config.intensity);  // Set LED intensity based on the config
    mqtt_topic_str = hostname_m + String(mqtt_topic);
    
    LOG_INFO("Device configured: " + hostname_m);
    LOG_DEBUG("Language: " + lang_weather);
    LOG_DEBUG("DHT22: " + String(IS_DHT_CONNECTED ? "connected" : "disconnected"));
    LOG_DEBUG("MQTT: " + String(isMQTT ? "enabled" : "disabled"));

  } else {
    // Set default values if MAC address is not found in the config map
    lang_weather = "en";
    hostname_m = "ESP_Unknown";
    IS_DHT_CONNECTED = false;
    isWebClientNeeded = true;
    isReadWeather = true;
    nameofWatch = "New";
    
    LOG_WARNING("MAC address not found in config, using defaults");
  }

  LOG_INFO("Hostname: " + hostname_m);

  // Set days of the week based on language
  if (!lang_weather.compareTo("ru")) {
    String daysOfTheWeekT[7] = { "Вс.", "Пн.", "Вт.", "Ср.", "Чт.", "Пт.", "Сб." };
    for (int i = 0; i < 7; i++) daysOfTheWeek[i] = daysOfTheWeekT[i];
  } else {
    String daysOfTheWeekT[7] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
    for (int i = 0; i < 7; i++) daysOfTheWeek[i] = daysOfTheWeekT[i];
  }
}

// Function to verify Wi-Fi connection
void verifyWifi() {
  while (WiFi.status() != WL_CONNECTED || WiFi.localIP() == IPAddress(0, 0, 0, 0)) {
    WiFi.reconnect();
  }
}

// Function to get a two-digit number as a string (with leading zero if necessary)
String getNumberWithZerro(int dig) {
    return (dig < 10) ? "0" + String(dig, DEC) : String(dig, DEC);
}

// Wrapper function for drawing text on the display
void drawString(String tape) {
  drawStringMax(tape);
}
