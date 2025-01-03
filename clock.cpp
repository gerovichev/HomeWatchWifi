#include "clock.h"

// Singleton instance of the Clock class
Clock& Clock::getInstance() {
    static Clock instance;
    return instance;
}

// Static member variable
volatile bool Clock::runClock = false;

// Initialize the clock and start the timer
void Clock::init() {
    timer1.attach(5, TimerHandler);  // Attach timer with 5s interval
}

// Detach the timer
void Clock::detach() {
    timer1.detach();  // Stop the timer
}

// Main loop for handling the clock updates
void Clock::loop() {
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

// Timer handler function that triggers on each timer interrupt
void IRAM_ATTR Clock::TimerHandler() {
    runClock = true;
}

// Handles different cases for the clock counter to display various information
void Clock::handleClockCounter() {
    switch (clockCounter) {
        case 1:  printDateToScreen(); break;
        case 3:  printDayToScreen(); break;
        case 5:  weatherManager.printWeatherToScreen(); break;
        case 7:  weatherManager.printMaxTempToScreen(); break;
        case 9:  weatherManager.printPressureToScreen(); break;
        case 11: weatherManager.printHumidityToScreen(); break;
        case 13: weatherManager.printDescriptionWeatherToScreen(); break;
        case 15: currencyManager.displayUSDToScreen(); break;
        case 17: 
            currencyManager.displayEURToScreen(); 
            if (!IS_DHT_CONNECTED) {
                shouldStopLoop = true;
            }
            break;
        case 19:  dht22_manager.printHomeTemp(); break;
        case 21: 
            dht22_manager.printHumidity();
            shouldStopLoop = true;
            break;
        default:
            shouldStopLoop = true;
            break;
    }
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
