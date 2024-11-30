#include "main_process.h"

#include <ESP8266WiFi.h>

bool isRunWeather = false;

Ticker updateDataTicker;

// Timer interrupt handler to trigger weather and currency updates
void IRAM_ATTR runAllUpdates() {
    isRunWeather = true;
}

// Setup function, called once at startup
void setup() {
    Serial.begin(115200);
    delay(500);

    if (Serial) Serial.println(F("Start ..."));

    initPerDevice();
    matrixSetup();

    printText(F("Hello ") + nameofWatch);
    delay(1500);

    printText(version_prg);
    delay(2000);

    printText(F("Connect WIFI"));
    
    WIFISetup wifiSetup;
    wifiSetup.wifi_init();  // Initialize Wi-Fi

    printText(WiFi.localIP().toString());

    location_init();
    ntp_init();
    Dht22_manager& dht22_manager = Clock::getInstance().getDht22();
    dht22_manager.dht22Start();  // Start DHT22 sensor

    if (isOTAreq) {
        web_ota_init();
    }

    if (isMQTT) {
        setup_mqtt();  // Initialize MQTT client
    }

    webserver_init();  // Initialize web server

    timeNow = timeClient.getEpochTime();  // Get the current time

    isRunWeather = true;  // Set flag to trigger weather updates

    printCityToScreen();  // Display the city

    // Set up ticker to call services every 15 minutes
    updateDataTicker.attach(TIME_TO_CALL_SERVICES, runAllUpdates);
}

// Function to fetch weather and currency data
void fetchWeatherAndCurrency() {
    if (isRunWeather) {
        isRunWeather = false;
        if (Serial) Serial.println(F("Start detach"));
        
        detachInterrupt_clock_process();  // Detach clock interrupt
        enableWiFi();
        if (Serial) Serial.println(F("Detached"));
        yield();
        
        if (isOTAreq) {
            update_ota();  // Handle OTA updates
        }
        yield();

        if (Serial) Serial.println("Time: " + timeClient.getEpochTime());
        getTimezone();  // Update timezone info
        if (Serial) Serial.println(F("Get time zone finished"));    
        yield();

        Clock::getInstance().getWeatherManager().readWeather();  // Fetch weather data
        yield();

        Clock::getInstance().getCurrencyManager().initialize();  // Initialize currency data
        yield();

        setIntensityByTime(timeNow);  // Adjust display intensity based on time
        yield();

        if (isMQTT) {
            if (!client.connected()) {
                reconnect();  // Reconnect to MQTT broker if needed
            }
            client.loop();  // Keep MQTT client running
            publish_temperature();  // Publish temperature to MQTT
        }
        yield();

        disableWiFi();

        init_clock_process();  // Reinitialize clock process
        yield();
    }
}

// Main loop, called repeatedly
void loop() {
    if (displayAnimate()) {
        timeClient.update();  // Update the time from NTP server
        timeNow = timeClient.getEpochTime();

        fetchWeatherAndCurrency();  // Fetch weather and currency data
        clock_loop();  // Handle clock logic
        realDisplayText();  // Update the display
    }

    webClientHandle();  // Handle web server requests
    ESP.wdtFeed();  // Feed the watchdog timer
}

// Function to enable Wi-Fi (if disabled)
void enableWiFi() {
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.forceSleepWake();
    WiFi.begin(); // Reconnect using saved credentials
    Serial.print("Reconnecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("\nWi-Fi reconnected!");
  } else {
    Serial.println("Wi-Fi already connected.");
  }
}

// Function to disable Wi-Fi
void disableWiFi() {
  WiFi.disconnect(true, false);
  WiFi.mode(WIFI_OFF);
  Serial.println("Wi-Fi disabled to save power.");
}
