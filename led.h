#pragma once

#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
#include <vector>
#include "fonts.h"
#include "global.h"


// Define the number of devices in the chain and the hardware interface
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4

#define CLK_PIN 14   // Clock pin (D5)
#define DATA_PIN 13  // Data pin (D7)
#define CS_PIN 5     // Chip select pin (D1)

#define LED_MAX_BUF 512  // Size of the LED buffer

bool newMessageAvailable = false;

// Hardware SPI connection
MD_Parola M = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

class LEDBuffer {
public:
  LEDBuffer(size_t size)
    : bufferSize(size) {
    buffer.resize(bufferSize);
    clearBuffer();
  }

  void clearBuffer() {
    std::fill(buffer.begin(), buffer.end(), '\0');
  }

  char* getBuffer() {
    return buffer.data();
  }

  size_t getBufferSize() const {
    return bufferSize;
  }

private:
  std::vector<char> buffer;
  size_t bufferSize;
};

// Sets the intensity of the display
void setIntensity(byte intensity) {
  M.setIntensity(intensity);
}

// Function to convert time_t to a readable string format
String formatTime(time_t rawTime) {
  int hours = (rawTime % 86400L) / 3600;
  int minutes = (rawTime % 3600) / 60;
  int seconds = rawTime % 60;
  
  char timeStr[20];
  sprintf(timeStr, "%02d:%02d:%02d", hours, minutes, seconds);
  
  return String(timeStr);
}

// Sets the intensity based on the current time
void setIntensityByTime(time_t timeNow) {
  int intensity = (timeNow > sunrise && timeNow < sunset) ? 2 : 0;

  if (Serial) 
  {
    Serial.println(F("sunrise: ") + formatTime(sunrise)); 
    Serial.println(F("Time: ") + formatTime(timeNow));
    Serial.println(F("sunset: ") + formatTime(sunset));

    Serial.printf("intensity %d\n", intensity);
  }
  M.setIntensity(intensity);
}

// Converts UTF-8 to Russian characters
String utf2rus(const String& source) {
  String target;
  target.reserve(source.length()); // Pre-allocate memory to avoid dynamic resizing

  for (int i = 0, k = source.length(); i < k; ++i) {
    unsigned char n = source[i];
    if (n >= 0xC0) {
      switch (n) {
        case 0xD0: {
          n = source[++i];
          if (n == 0x81) { n = 0xA8; }
          else if (n >= 0x90 && n <= 0xBF) { n += 0x30; }
          break;
        }
        case 0xD1: {
          n = source[++i];
          if (n == 0x91) { n = 0xB8; }
          else if (n >= 0x80 && n <= 0x8F) { n += 0x70; }
          break;
        }
      }
    }
    target += char(n);
  }

  return target;
}

// Global variable to store the last displayed text for comparison
String lastDisplayedText = "";

// Draws a string on the LED display
void drawStringMax(const String& tape) {
  // Convert the input text for display and check if it's different from the last displayed text
  String convertedText = utf2rus(tape);
  if (convertedText != lastDisplayedText) {
    lastDisplayedText = convertedText; // Update the last displayed text
    newMessageAvailable = true;
  }
}

#define SCROLL_SPEED 50
#define PAUSE_TIME 1000

// Displays the text on the LED display
void realDisplayText() {

  if (M.displayAnimate() && newMessageAvailable) {
    newMessageAvailable = false;
    M.displayReset();
    if (Serial) Serial.println(lastDisplayedText);
    M.displayClear();  // Clear the display before setting new text

    if (lastDisplayedText.length() > 5) {
      M.displayText(lastDisplayedText.c_str(), PA_LEFT, SCROLL_SPEED, PAUSE_TIME, PA_SCROLL_LEFT, PA_NO_EFFECT);
    } else {
      M.displayText(lastDisplayedText.c_str(), PA_CENTER, SCROLL_SPEED, PAUSE_TIME, PA_PRINT, PA_NO_EFFECT);
    }
  }
}

// Animates the display
bool displayAnimate() {
  return M.displayAnimate();
}

// Initializes the LED matrix display
void matrixSetup() {
  M.begin();
  M.displayClear();
  M.displaySuspend(false);
  M.setInvert(false);
  M.setFont(CRMrusTxt);
  M.setIntensity(0);
}

// Prints the given text on the LED display
void printText(String text) {
  if (Serial) Serial.println(text);
  char dataText[LED_MAX_BUF];
  utf2rus("     " + text).toCharArray(dataText, LED_MAX_BUF);

  int textLength = strlen(dataText) - 6;

  if (textLength > 5) {
    for (int i = 0; i < textLength; i++) {
      M.print(&dataText[i]);
      delay(250);
    }
  } else {
    M.print(&dataText[5]);
  }
}
