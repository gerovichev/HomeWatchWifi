#include "global.h"
#include "max72xx.h"
#include "wifimanager.h"
#include "geo_place.h"
#include "ntp_g.h"
#include "dht22.h"
#include "weather.h"
#include "web_ota.h"
#include "ota.h"
#include "clock_process.h"
#include "webclient.h"

void setup() {

  // put your setup code here, to run once:
  Serial.begin(115200);

  delay(500);

  Serial.println("Start ...");

  matrixSetup();

  initPerDevice();

  drawString("Hello " + nameofWatch, 0);
  delay(500);
  drawString(version_prg, 0);
  delay(500);
  drawString("Connect WIFI", 0);
  wifi_init();

  drawString(WiFi.localIP().toString(), 0);
  delay(500);

  location_init();
  delay(500);

  ntp_init();

  //weather_init();
  delay(500);

  dht22Start();

  if (isOTAreq) {
    update_ota();
    ota_init();
  }

  webserver_init();



  drawString("Start", 0);




  timeNow = timeClient.getEpochTime();
  //setIntensityByTime(timeNow);
  isRunWeather = true;

  init_clock_process();
}

void loop() {
  verifyWifi();
  timeClient.update();
  timeNow = timeClient.getEpochTime() - offset;
  // Serial.println(timeClient.formattedTime("%d. %B %Y")); // dd. Mmm yyyy
  // Serial.println(timeClient.formattedTime("%A %T")); // Www hh:mm:ss
  //delay(1000);

  clock_loop();


  if (isRunWeather) {
    detachInterrupt_clock_process();

    Serial.println(timeClient.getEpochTime());  // dd. Mmm yyyy


    getTimezone();

    if (isOTAreq) {
      ArduinoOTA.handle();
      update_ota();
    }

    readWeather();

    data_USD_value = readCurrency(pathCurrencyUSD);

    data_EUR_value = readCurrency(pathCurrencyEUR);


    setIntensityByTime(timeNow);

    isRunWeather = false;
    init_clock_process();
  }

  webClientHandle();
}
