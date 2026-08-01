#include "main_process.h"
#include "device_state.h"
#include "calendar_manager.h"

#include "constants.h"
#include <ESP8266WiFi.h>
#include <TimeLib.h> // For advanced time manipulation

volatile bool isRunWeather = false;

Ticker updateDataTicker;

// WiFi reconnection tracking
static unsigned long lastWiFiCheck = 0;
constexpr unsigned long WIFI_CHECK_INTERVAL_MS = 60000;  // Check WiFi every 60 seconds

// Timer interrupt handler to trigger weather and currency updates
void IRAM_ATTR runAllUpdates() { isRunWeather = true; }

// Setup function, called once at startup
void setup() {
  Logger::getInstance().begin(115200);
  Logger::getInstance().setLogLevel(LOG_LEVEL_NONE); // Set desired log level

  LOG_INFO_F("Starting HomeWatchWifi...");
  LOG_INFO_FMT("Version: %s", version_prg.c_str());

  initPerDevice();
  matrixSetup();

  // Optimize String concatenation
  String helloMsg;
  helloMsg.reserve(10 + DeviceState::getInstance().getWatchName().length());
  helloMsg = F("Hello ");
  helloMsg += DeviceState::getInstance().getWatchName();
  displayTextInSetup(helloMsg);

  displayTextInSetup(version_prg);

  LOG_INFO_F("Initializing WiFi...");
  displayTextInSetup(F("Connect WIFI"));

  WIFISetup wifiSetup;
  wifiSetup.wifi_init(); // Initialize Wi-Fi

  displayTextInSetup(WiFi.localIP().toString());

  LOG_INFO_FMT("IP Address: %s", WiFi.localIP().toString().c_str());

  LOG_INFO_F("Initializing location services...");
  location_init();

  LOG_INFO_F("Initializing NTP time...");
  ntp_init();

  LOG_INFO_F("Starting DHT22 sensor...");
  Dht22_manager &dht22_manager = Clock::getInstance().getDht22();
  dht22_manager.dht22Start(); // Start DHT22 sensor

  if (DeviceState::getInstance().isOtaRequired()) {
    LOG_INFO_F("Initializing OTA updates...");
    web_ota_init();
  }

  if (DeviceState::getInstance().isMqttEnabled()) {
    LOG_INFO_F("Initializing MQTT client...");
    setup_mqtt(); // Initialize MQTT client
  }

  timeNow = timeClient.getEpochTime(); // Get the current time

  // Initialize calendar events
  LOG_INFO_F("Initializing calendar events...");
  Clock::getInstance().getCalendarManager().readCalendarEvents();

  isRunWeather = true; // Set flag to trigger weather updates

  // printCityToScreen();  // Display the city

  // Set up ticker to call services every configured interval
  updateDataTicker.attach(Timing::DATA_UPDATE_INTERVAL_SEC, runAllUpdates);

  LOG_INFO_F("Setup completed successfully!");
}

// Function to fetch weather and currency data
void fetchWeatherAndCurrency() {
  if (isRunWeather) {
    isRunWeather = false;
    LOG_INFO_F("Starting data update cycle...");

    detachInterrupt_clock_process(); // Detach clock interrupt
    LOG_DEBUG_F("Clock process detached");

    //enableWiFi();

    if (WiFi.status() == WL_CONNECTED) {
      LOG_DEBUG_F("WiFi connected, updating services...");

      if (DeviceState::getInstance().isOtaRequired()) {
        LOG_DEBUG_F("Checking for OTA updates...");
        update_ota(); // Handle OTA updates
      }

      LOG_DEBUG_F("Updating location (if 1 hour has passed)...");
      static unsigned long lastLocationCheck = 0;
      if (lastLocationCheck == 0 || millis() - lastLocationCheck > 3600000UL) { // 1 hour
        location_init();
        lastLocationCheck = millis();
        // Handle edge case where millis() was extremely close to 0 on boot
        if (lastLocationCheck == 0) lastLocationCheck = 1;
      }

      if (DeviceState::getInstance().isMqttEnabled()) {
        if (!client.connected()) {
          LOG_WARNING_F("MQTT disconnected, reconnecting...");
          reconnect(); // Reconnect to MQTT broker if needed
        }
        client.loop(); // Keep MQTT client running
        LOG_DEBUG_F("Publishing temperature to MQTT...");
        publish_temperature(); // Publish temperature to MQTT
      }

      LOG_DEBUG_F("Updating time from NTP...");
      timeClient.update(); // Update the time from NTP server
      timeNow = timeClient.getEpochTime();
      setTime(timeNow);
      LOG_VERBOSE_FMT("Current epoch time: %lu", timeNow);

      LOG_DEBUG_F("Updating timezone...");
      getTimezone(); // Update timezone info

      LOG_DEBUG_F("Fetching weather data...");
      Clock::getInstance()
          .getWeatherManager()
          .readWeather(); // Fetch weather data

      LOG_DEBUG_F("Fetching currency rates...");
      Clock::getInstance()
          .getCurrencyManager()
          .initialize(); // Initialize currency data

      LOG_DEBUG_F("Updating calendar events...");
      CalendarManager &calendarManager = Clock::getInstance().getCalendarManager();
      if (calendarManager.shouldUpdateToday()) {
        calendarManager.readCalendarEvents(); // Update calendar events
      }

      LOG_DEBUG_F("Adjusting display intensity...");
      setIntensityByTime(timeNow); // Adjust display intensity based on time

      LOG_INFO_F("Data update cycle completed successfully");
    } else {
      LOG_ERROR_F("WiFi not connected, skipping data update");
    }

    //disableWiFi();

    init_clock_process(); // Reinitialize clock process
  }
}

// Main loop, called repeatedly
void loop() {
  // Check for minute change on every iteration (independent of displayAnimate)
  Clock::getInstance().checkMinuteChange();

  // Periodically check WiFi connection and attempt reconnection if needed.
  // Bug fix #4: the previous call to attemptReconnect() blocked loop() for up
  // to WIFI_CONNECT_TIMEOUT_MS (10 s) with no watchdog feeding, risking a
  // hardware-watchdog reset. Instead, we kick off a non-blocking reconnect:
  // WiFi auto-reconnect (enabled in wifi_init) will finish the work in the
  // background. We just log and nudge the stack.
  unsigned long currentTime = millis();
  if (currentTime - lastWiFiCheck > WIFI_CHECK_INTERVAL_MS) {
    lastWiFiCheck = currentTime;

    if (WiFi.status() != WL_CONNECTED) {
      LOG_WARNING_F("WiFi disconnected, triggering non-blocking reconnect...");
      // Keep reconnect non-blocking and avoid credential rewrites.
      WiFi.mode(WIFI_STA);
      WiFi.reconnect();
      // Do NOT block here — auto-reconnect fires in the background.
      // The data update cycle already guards all HTTP calls with a WiFi check.
    }
  }

  if (displayAnimate()) {

    fetchWeatherAndCurrency(); // Fetch weather and currency data
    clock_loop();              // Handle clock logic
    realDisplayText();         // Update the display
  }

  ESP.wdtFeed(); // Feed the watchdog timer
}

// Function to enable Wi-Fi (if disabled)
void enableWiFi() {
  // Check if already connected
  if (WiFi.status() == WL_CONNECTED) {
    LOG_DEBUG_F("WiFi already connected");
    return;
  }

  WiFi.begin(); // Reconnect using saved credentials
  LOG_INFO_F("Reconnecting to WiFi...");

  unsigned long startAttemptTime = millis();
  int dotCount = 0;

  while (WiFi.status() != WL_CONNECTED) {
    // Check for timeout
    if (millis() - startAttemptTime > Timing::WIFI_TIMEOUT_MS) {
      LOG_ERROR_F("WiFi connection timeout");
      return;
    }

    // Visual feedback every 500ms
    if (++dotCount % 5 == 0) {
      LOG_VERBOSE_F("Still connecting...");
    }
    yield(); // Allow other tasks to run instead of blocking delay
  }

  // Connection successful
  LOG_INFO_FMT("WiFi reconnected! IP: %s", WiFi.localIP().toString().c_str());
}

// Function to disable Wi-Fi
void disableWiFi() {
  WiFi.disconnect(true, false);
  WiFi.mode(WIFI_OFF);
  LOG_INFO_F("WiFi disabled to save power");
}
