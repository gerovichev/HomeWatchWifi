//#include "WiFiManagerWrapper.h"
#include "global.h"
#include "led.h"
#include "wifimanager.h"
#include "geo_place.h"
#include "ntp_g.h"
#include "dht22.h"
#include "weather.h"
#include "web_ota.h"
#include "clock_process.h"
//#include "WebServerHandlers.h"
#include "webclient.h"
#include <Ticker.h>
#include "MQTTClient.h"

#define TIME_TO_CALL_SERVICES 900

Ticker updateDataTicker;
bool isRunWeather = false;
// Init ESP8266 timer 0
//ESP8266Timer ITimerWeather;


//WiFiManagerWrapper wifiManagerWrapper;

void IRAM_ATTR runAllUpdates() {
  isRunWeather = true;
}

void setup() {

  // put your setup code here, to run once:
  Serial.begin(115200);

  delay(500);

  Serial.println("Start ...");



  initPerDevice();

  matrixSetup();

  printText("Hello " + nameofWatch);
  delay(1500);

  printText(version_prg);
  delay(2000);

  printText("Connect WIFI");
  //delay(500);

  //wifiManagerWrapper.init();
  wifi_init();

  printText(WiFi.localIP().toString());
  //delay(1500);

  location_init();
  //delay(500);

  ntp_init();

  //weather_init();
  //delay(500);
  //printText("Start");
  dht22Start();

  if (isOTAreq) {
    web_ota_init();
  }

  if (isMQTT) {
    setup_mqtt();
  }

  webserver_init();

  //printText("Start 1");

  timeNow = timeClient.getEpochTime();

  isRunWeather = true;

  //init_clock_process();

  //printText("Start 2");

  printCityToScreen();

  // Set up Ticker to update weather and currency every hour (3600 seconds)
  updateDataTicker.attach(TIME_TO_CALL_SERVICES, runAllUpdates);

  //ITimer.attachInterruptInterval(TIMER_INTERVAL_MS * 900000/15, runAllUpdates);
  //drawStringMax("Start", 0);

  // Initial data fetch
  //fetchWeatherAndCurrency();
  //printText("Start");
}



void fetchWeatherAndCurrency() {

  /* if (!updateDataTicker.active()) {
    Serial.println("Ticker is not active");
    // Set up Ticker to update weather and currency every hour (3600 seconds)
    updateDataTicker.attach(TIME_TO_CALL_SERVICES, runAllUpdates);
  }*/
  if (isRunWeather) {

    isRunWeather = false;
    Serial.println("Start detach");
    
    //updateDataTicker.detach();
    detachInterrupt_clock_process();
    Serial.println("Detached");
    yield();
  
    if (isOTAreq) {
      //ArduinoOTA.handle();
      update_ota();
    }
    yield();

    Serial.println("Time: " + timeClient.getEpochTime());  // dd. Mmm yyyy
    getTimezone();
    Serial.println("Get time zone finished");    
    yield();

    readWeather();
    yield();

    currency_init();
    yield();

    setIntensityByTime(timeNow);
    yield();

    if (isMQTT) {
      if (!client.connected()) {
        reconnect();
      }
      client.loop();
      publish_temperature(homeTemp);
    }
    yield();

    //updateDataTicker.attach(TIME_TO_CALL_SERVICES, runAllUpdates);
    init_clock_process();
    yield();
  }
}

void loop() {

  //verifyWifi();

  if (displayAnimate()) {
    //Serial.println("displayAnimate() = true");

    timeClient.update();
    timeNow = timeClient.getEpochTime();// - offset;

    fetchWeatherAndCurrency();
    //printTimeToScreen();

    clock_loop();
    realDisplayText();
  }
  webClientHandle();
}
