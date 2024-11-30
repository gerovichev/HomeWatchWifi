#pragma once

#include "weather_manager.h"
#include "dht22_manager.h"
#include "currency_manager.h"
#include <Ticker.h>
#include "TimeManager.h"
#include "secret.h"

constexpr int TIMER_INTERVAL_MS = 1000;
constexpr int DISPLAY_CYCLE_LENGTH = 21;

class Clock {
public:
    static Clock& getInstance(); // Singleton access to instance

    void init();
    void detach();
    void loop();
    Dht22_manager& getDht22();
    WeatherManager& getWeatherManager();
    CurrencyManager& getCurrencyManager();

private:
    Clock() = default; // Private constructor for singleton pattern
    
    // Delete copy constructor and assignment operator
    Clock(const Clock &) = delete;
    Clock &operator=(const Clock &) = delete;

    Dht22_manager dht22_manager;

    static void IRAM_ATTR TimerHandler();
    void handleClockCounter();

    bool shouldStopLoop = false;
    int clockCounter = 0;
    static volatile bool runClock;
    Ticker timer1;
    WeatherManager weatherManager;

    CurrencyManager currencyManager;
};

// Initialize clock process functions
void init_clock_process();
void detachInterrupt_clock_process();
void clock_loop();
