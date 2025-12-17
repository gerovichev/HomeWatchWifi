#include "clock.h"
#include "constants.h"
#include "global_config.h"
#include "logger.h"
#include <TimeLib.h>

// Singleton instance of the Clock class
Clock &Clock::getInstance() {
  static Clock instance;
  return instance;
}

// Initialize the clock
void Clock::init() {
  buildDisplaySequence();   // Build display sequence
  currentDisplayIndex = 0;  // Reset index
  lastDisplayedMinute = -1; // Initialize minute tracking
  lastChangeTime = millis();
  isTransitioning = false;
}

// Detach - no longer needed with millis approach but kept for interface
// compatibility
void Clock::detach() {
  // No timer to detach
}

// Main loop for handling the clock updates
void Clock::loop() {
  unsigned long currentTime = millis();

  // Check if enough time has passed since last change
  bool timeExpired =
      (currentTime - lastChangeTime) >= (Timing::CLOCK_INTERVAL_SEC * 1000UL);

  // Check if animation is finished (returns true if done)
  // We only proceed if BOTH time has expired AND animation is done
  if (timeExpired && displayAnimate()) {

    // Move to next element or start from beginning
    currentDisplayIndex++;
    if (currentDisplayIndex >= displaySequenceLength) {
      currentDisplayIndex = 0;
      LOG_VERBOSE_F("Display cycle completed, restarting");
    }

    // Execute current action from sequence
    if (displaySequenceLength > 0) {
      executeDisplayAction();
      lastChangeTime = currentTime; // Reset timer
    }
  }
}

// Executes current action from sequence
void Clock::executeDisplayAction() {
  if (currentDisplayIndex < displaySequenceLength &&
      displaySequence[currentDisplayIndex] != nullptr) {
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
void Clock::displayWeatherDescription() {
  weatherManager.printDescriptionWeatherToScreen();
}
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
  LOG_DEBUG("Display sequence built with " + String(displaySequenceLength) +
            " items");
}

// Checks for minute change and switches to time display
void Clock::checkMinuteChange() {
  // Get current minute using TimeLib (more reliable)
  int currentMinute = minute();

  // If minute changed and we're not showing time
  if (lastDisplayedMinute != -1 && currentMinute != lastDisplayedMinute) {
    // Check if we're currently showing time
    bool isCurrentlyShowingTime =
        (displaySequence[currentDisplayIndex] == &Clock::displayTime);

    if (!isCurrentlyShowingTime) {
      // Find next time index and switch to it
      int nextTimeIndex = findNextTimeIndex();
      if (nextTimeIndex != -1) {
        currentDisplayIndex = nextTimeIndex;
        executeDisplayAction(); // Immediately show time
        lastChangeTime =
            millis(); // Reset timer so this display stays for full duration
        LOG_VERBOSE("Minute changed, switched to time display");
      }
    } else {
      // If already showing time, just update it
      executeDisplayAction();
      lastChangeTime = millis(); // Reset timer
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

Dht22_manager &Clock::getDht22() { return dht22_manager; }

WeatherManager &Clock::getWeatherManager() { return weatherManager; }

CurrencyManager &Clock::getCurrencyManager() { return currencyManager; }

// Initialize clock process
void init_clock_process() { Clock::getInstance().init(); }

// Detach the timer interrupt for the clock process
void detachInterrupt_clock_process() { Clock::getInstance().detach(); }

// Main clock loop
void clock_loop() { Clock::getInstance().loop(); }
