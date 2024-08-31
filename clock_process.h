#pragma once

//#include <ESP8266TimerInterrupt.h>
#include "weather.h"
#include "dht22.h"
#include "currencyMy.h"
#include <Ticker.h>

constexpr int TIMER_INTERVAL_MS = 1000;
constexpr int DISPLAY_CYCLE_LENGTH = 21;

// Init ESP8266 timer 0
//ESP8266Timer ITimer;



class Clock {
public:
    void init() {
        //ITimer.attachInterruptInterval(TIMER_INTERVAL_MS * 5000, TimerHandler);
        timer1.attach(5, TimerHandler);
    }

    void detach() {
        timer1.detach();
        //ITimer.detachInterrupt();
    }

    void loop() {
        if (runClock) {
            runClock = false;
            if (Serial) Serial.println(clockCounter);

            if (clockCounter % 2 == 0) {
                printTimeToScreen();
            } else {
                handleClockCounter();
            }

            clockCounter++;
            if (shouldStopLoop || clockCounter > DISPLAY_CYCLE_LENGTH) {
                clockCounter = 0;
                shouldStopLoop = false;
            }
        }
    }

private:
    static void IRAM_ATTR TimerHandler() {
        runClock = true;
    }

    void handleClockCounter() {
        switch (clockCounter) {
            case 1: printDateToScreen(); break;
            case 3: printDayToScreen(); break;
            case 5: printWeatherToScreen(); break;
            case 7: printMaxTempToScreen(); break;
            case 9: printPressureToScreen(); break;
            case 11: printHumidityToScreen(); break;
            case 13: printDescriptionWeatherToScreen(); break;
            case 15: printCurrencyUSDToScreen(); break;
            case 17: 
                printCurrencyEURToScreen(); 
                if (!IS_DHT_CONNECTED) {
                    shouldStopLoop = true;
                }
                break;
            case 19: printHomeTemp(); break;
            case 21:
                printHumidity();
                shouldStopLoop = true;
                break;
            default: 
                shouldStopLoop = true; 
                break;
        }
    }

    static Clock instance;
    bool shouldStopLoop = false;
    int clockCounter = 0;
    static volatile bool runClock;
    Ticker timer1;
};

Clock Clock::instance;
volatile bool Clock::runClock = false;

Clock clockPr;

void init_clock_process() {
    clockPr.init();
}

void detachInterrupt_clock_process() {
    clockPr.detach();
}

void clock_loop() {
    clockPr.loop();
}
