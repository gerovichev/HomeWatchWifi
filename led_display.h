#pragma once

#include <MD_Parola.h>
#include "fonts.h"
#include "global_config.h"

// Define the number of devices in the chain and the hardware interface
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4

#define CLK_PIN 14   // Clock pin (D5)
#define DATA_PIN 13  // Data pin (D7)
#define CS_PIN 5     // Chip select pin (D1)

// LED_MAX_BUF moved to constants.h as Buffer::LED_BUFFER_SIZE

extern bool newMessageAvailable;

// Function declarations
void setIntensity(byte intensity);
String formatTime(time_t rawTime);
void setIntensityByTime(time_t timeNow);
String utf2rus(const String& source);
void drawStringMax(const String& tape);
void realDisplayText();
void forceDisplayText();
void waitForAnimation();
void displayTextInSetup(const String& text);
bool displayAnimate();
void matrixSetup();
void printText(String text);
