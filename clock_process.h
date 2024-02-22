#pragma once

#include <ESP8266TimerInterrupt.h>

#include "weather.h"
#include "dht22.h"
#include "currencyMy.h"

//#include "ntp.h"

#define TIMER_INTERVAL_MS 1000

// Init ESP8266 timer 0
ESP8266Timer ITimer;

int clock_counter = 0;

bool runClock = false;

bool isRunWeather = false;

int countWeather = 0;

void isNeedRunWeather() {
  if (countWeather >= 15 * 12) {
    countWeather = 0;
    isRunWeather = true;
  }
  countWeather++;
  Serial.print("countWeather: ");
  Serial.println(countWeather);
}

//=======================================================================
void ICACHE_RAM_ATTR TimerHandler() {
  runClock = true;
  isNeedRunWeather();
}

void clock_loop() {

  if (runClock) {
    
    runClock = false;

    bool stop_loop = false;

    Serial.println(clock_counter);

    if (clock_counter % 2 == 0) {

      //Serial.println(clock_counter);

      printTimeToScreen();

    } else {
      switch (clock_counter) {
        case 1:
          printDateToScreen();
          break;
        case 3:
          printDayToScreen();
          break;
        case 5:
          printWeatherToScreen();
          break;
        case 7:
          printMaxTempToScreen();
          break;
        case 9:
          printPressureToScreen();
          break;
        case 11:
          printHumidityToScreen();
          break;
        case 13:
          printDescriptionWeatherToScreen();
          break;
        case 15:
          printCurrencyUSDToScreen();
          break;
        case 17:
          printCurrencyEURToScreen();
          if (!IS_DHT_CONNECTED) {
            stop_loop = true;
          }
          break;
        case 19:
          printHomeTemp();
          break;
        case 21:
          printHumidity();
          stop_loop = true;
          break;
        default:
          stop_loop = true;

          break;

      }
    }

    clock_counter++;

    if (stop_loop || clock_counter > 21) {
      clock_counter = 0;
    }
    

  }
}

void init_clock_process() {
  ITimer.attachInterruptInterval(TIMER_INTERVAL_MS * 7000, TimerHandler);
}

void detachInterrupt_clock_process() {
  ITimer.detachInterrupt();
}
