#pragma once

#include "weather_manager.h"
#include "dht22_manager.h"
#include "currency_manager.h"
#include <Ticker.h>
#include "TimeManager.h"
#include "secret.h"

// TIMER_INTERVAL_MS and DISPLAY_CYCLE_LENGTH moved to constants.h

class Clock {
public:
    static Clock& getInstance(); // Singleton access to instance

    void init();
    void detach();
    void loop();
    void checkMinuteChange();  // Public method to check for minute change
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
    void executeDisplayAction();
    void buildDisplaySequence();
    int findNextTimeIndex();
    
    // Wrapper methods for display
    void displayTime();
    void displayDate();
    void displayDay();
    void displayWeather();
    void displayMaxTemp();
    void displayPressure();
    void displayWeatherHumidity();
    void displayWeatherDescription();
    void displayUSD();
    void displayEUR();
    void displayHomeTemp();
    void displayHomeHumidity();

    int currentDisplayIndex = 0;
    int lastDisplayedMinute = -1;  // Track last displayed minute
    static volatile bool runClock;
    Ticker timer1;
    WeatherManager weatherManager;
    CurrencyManager currencyManager;
    
    // Array of function pointers for display
    typedef void (Clock::*DisplayAction)();
    DisplayAction displaySequence[24];  // Enough for all sequence elements
    int displaySequenceLength = 0;
};

// Initialize clock process functions
void init_clock_process();
void detachInterrupt_clock_process();
void clock_loop();
