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
        case 5:  printWeatherToScreen(); break;
        case 7:  printMaxTempToScreen(); break;
        case 9:  printPressureToScreen(); break;
        case 11: printHumidityToScreen(); break;
        case 13: printDescriptionWeatherToScreen(); break;
        case 15: printCurrencyUSDToScreen(); break;
        case 17: 
            printCurrencyEURToScreen(); 
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
