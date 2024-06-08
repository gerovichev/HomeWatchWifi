#include "global.h"
#include "led.h"
#include "wifimanager.h"
#include "geo_place.h"
#include "ntp_g.h"
#include "dht22.h"
#include "weather.h"
#include "web_ota.h"
#include "ota.h"
#include "clock_process.h"
#include "webclient.h"
#include <Ticker.h>

Ticker updateDataTicker;
bool isRunWeather = false;

void setup() {

  // put your setup code here, to run once:
  Serial.begin(115200);

  delay(500);

  Serial.println("Start ...");



  initPerDevice();

  matrixSetup();

  printText("Hello " + nameofWatch);
  //delay(1500);

  printText(version_prg);
  //delay(2000);

  printText("Connect WIFI");
  //delay(500);

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
    //update_ota();
    ota_init();
  }

  webserver_init();

  //printText("Start 1");


  timeNow = timeClient.getEpochTime();

  isRunWeather = true;

  init_clock_process();

  //printText("Start 2");

  printCityToScreen();

  // Set up Ticker to update weather and currency every hour (3600 seconds)
  updateDataTicker.attach(900, runAllUpdates);
  //drawStringMax("Start", 0);

  // Initial data fetch
  fetchWeatherAndCurrency();
  //printText("Start");
}

void runAllUpdates() {
  isRunWeather = true;
}

void fetchWeatherAndCurrency() {
  if (isRunWeather) {
    Serial.println("Start detach");
    detachInterrupt_clock_process();
    Serial.println("Detached");
    Serial.println("Time: " + timeClient.getEpochTime());  // dd. Mmm yyyy


    getTimezone();
    Serial.println("Get time zone finished");
    if (isOTAreq) {
      ArduinoOTA.handle();
      update_ota();
    }

    readWeather();

    currency_init();

    setIntensityByTime(timeNow);

    isRunWeather = false;
    init_clock_process();
  }
}

void loop() {

  //verifyWifi();

  if (displayAnimate()) {
    //Serial.println("displayAnimate() = true");

    fetchWeatherAndCurrency();
    //printTimeToScreen();


    timeClient.update();
    timeNow = timeClient.getEpochTime() - offset;

    clock_loop();

  } else {
    //Serial.println("displayAnimate() = false");
  }

  realDisplayText();

  webClientHandle();
}
