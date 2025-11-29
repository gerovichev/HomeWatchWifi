#include "clock.h"
#include "logger.h"
#include "constants.h"
#include "global_config.h"
#include <TimeLib.h>

// Singleton instance of the Clock class
Clock& Clock::getInstance() {
    static Clock instance;
    return instance;
}

// Static member variable
volatile bool Clock::runClock = false;

// Initialize the clock and start the timer
void Clock::init() {
    buildDisplaySequence();  // Build display sequence
    currentDisplayIndex = 0;  // Reset index
    lastDisplayedMinute = -1;  // Initialize minute tracking
    timer1.attach(Timing::CLOCK_INTERVAL_SEC, TimerHandler);  // Attach timer with configured interval
}

// Detach the timer
void Clock::detach() {
    timer1.detach();  // Stop the timer
}

// Main loop for handling the clock updates
void Clock::loop() {
    if (runClock) {
        runClock = false;
        
        // Execute current action from sequence
        if (displaySequenceLength > 0) {
            executeDisplayAction();
            
            // Move to next element or start from beginning
            currentDisplayIndex++;
            if (currentDisplayIndex >= displaySequenceLength) {
                currentDisplayIndex = 0;
                LOG_VERBOSE_F("Display cycle completed, restarting");
            }
        }
    }
}

// Timer handler function that triggers on each timer interrupt
void IRAM_ATTR Clock::TimerHandler() {
    runClock = true;
}

// Executes current action from sequence
void Clock::executeDisplayAction() {
    if (currentDisplayIndex < displaySequenceLength && displaySequence[currentDisplayIndex] != nullptr) {
        (this->*displaySequence[currentDisplayIndex])();
    }
}

// Wrapper methods for display (used as function pointers)
void Clock::displayTime() { printTimeToScreen(); }
void Clock::displayDate() { printDateToScreen(); }
void Clock::displayDay() { printDayToScreen(); }
void Clock::displayWeather() { weatherManager.printWeatherToScreen(); }
void Clock::displayMaxTemp() { weatherManager.printMaxTempToScreen(); }
void Clock::displayPressure() { weatherManager.printPressureToScreen(); }
void Clock::displayWeatherHumidity() { weatherManager.printHumidityToScreen(); }
void Clock::displayWeatherDescription() { weatherManager.printDescriptionWeatherToScreen(); }
void Clock::displayUSD() { currencyManager.displayUSDToScreen(); }
void Clock::displayEUR() { currencyManager.displayEURToScreen(); }
void Clock::displayHomeTemp() { dht22_manager.printHomeTemp(); }
void Clock::displayHomeHumidity() { dht22_manager.printHumidity(); }

// Builds display sequence based on available data
void Clock::buildDisplaySequence() {
    int index = 0;
    
    // Always show time and date
    displaySequence[index++] = &Clock::displayTime;
    displaySequence[index++] = &Clock::displayDate;
    displaySequence[index++] = &Clock::displayTime;
    displaySequence[index++] = &Clock::displayDay;
    
    // Weather
    displaySequence[index++] = &Clock::displayTime;
    displaySequence[index++] = &Clock::displayWeather;
    displaySequence[index++] = &Clock::displayTime;
    displaySequence[index++] = &Clock::displayMaxTemp;
    displaySequence[index++] = &Clock::displayTime;
    displaySequence[index++] = &Clock::displayPressure;
    displaySequence[index++] = &Clock::displayTime;
    displaySequence[index++] = &Clock::displayWeatherHumidity;
    displaySequence[index++] = &Clock::displayTime;
    displaySequence[index++] = &Clock::displayWeatherDescription;
    
    // Currency
    displaySequence[index++] = &Clock::displayTime;
    displaySequence[index++] = &Clock::displayUSD;
    displaySequence[index++] = &Clock::displayTime;
    displaySequence[index++] = &Clock::displayEUR;
    
    // Home sensors (only if connected)
    if (IS_DHT_CONNECTED) {
        displaySequence[index++] = &Clock::displayTime;
        displaySequence[index++] = &Clock::displayHomeTemp;
        displaySequence[index++] = &Clock::displayTime;
        displaySequence[index++] = &Clock::displayHomeHumidity;
    }
    
    displaySequenceLength = index;
    LOG_DEBUG("Display sequence built with " + String(displaySequenceLength) + " items");
}

// Checks for minute change and switches to time display
void Clock::checkMinuteChange() {
    // Get current minute using TimeLib (more reliable)
    int currentMinute = minute();
    
    // If minute changed and we're not showing time
    if (lastDisplayedMinute != -1 && currentMinute != lastDisplayedMinute) {
        // Check if we're currently showing time
        bool isCurrentlyShowingTime = (displaySequence[currentDisplayIndex] == &Clock::displayTime);
        
        if (!isCurrentlyShowingTime) {
            // Find next time index and switch to it
            int nextTimeIndex = findNextTimeIndex();
            if (nextTimeIndex != -1) {
                currentDisplayIndex = nextTimeIndex;
                executeDisplayAction();  // Immediately show time
                LOG_VERBOSE("Minute changed, switched to time display");
            }
        } else {
            // If already showing time, just update it
            executeDisplayAction();
        }
    }
    
    // Update last displayed minute
    lastDisplayedMinute = currentMinute;
}

// Finds next time index in display sequence
int Clock::findNextTimeIndex() {
    // Search for time starting from current index
    for (int i = currentDisplayIndex; i < displaySequenceLength; i++) {
        if (displaySequence[i] == &Clock::displayTime) {
            return i;
        }
    }
    
    // If not found, search from beginning of sequence
    for (int i = 0; i < currentDisplayIndex; i++) {
        if (displaySequence[i] == &Clock::displayTime) {
            return i;
        }
    }
    
    // If not found at all, return first element (which is always time)
    return 0;
}

Dht22_manager& Clock::getDht22()
{
  return dht22_manager;
}

WeatherManager& Clock::getWeatherManager()
{
  return weatherManager;
}

 CurrencyManager& Clock::getCurrencyManager()
 {
  return currencyManager;
 }

// Initialize clock process
void init_clock_process() {
    Clock::getInstance().init();
}

// Detach the timer interrupt for the clock process
void detachInterrupt_clock_process() {
    Clock::getInstance().detach();
}

// Main clock loop
void clock_loop() {
    Clock::getInstance().loop();
}
