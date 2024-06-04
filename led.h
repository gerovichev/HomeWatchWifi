#pragma once

#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
#include "fonts.h"

void setIntensity(byte intensity);

#include "global.h"

// Define the number of devices we have in the chain and the hardware interface
// NOTE: These pin numbers will probably not work with your hardware and may
// need to be adapted
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4

#define CLK_PIN 14
// - d5
#define DATA_PIN 13
// - d7
#define CS_PIN 5
// - d1

#define LED_MAX_BUF 512  // Размер буфера LED

bool newMessageAvailable = false;

// Hardware SPI connection
MD_Parola M = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

char data[LED_MAX_BUF];


void setIntensity(byte intensity) {
  M.setIntensity(intensity);
}

void setIntensityByTime(time_t timeNow) {
  int intensity = 0;
  if (timeNow > sunrise && timeNow < sunset) {
    intensity = 2;
    Serial.println("intensity 2");
  } else {
    intensity = 0;
    Serial.println("intensity 0");
  }


  M.setIntensity(intensity);
}

//UTF to RUS
String utf2rus(const String& source) {
  int i = 0, k = source.length();
  String target = String();
  unsigned char n;
  char m[2] = { '0', '\0' };
  while (i < k) {
    n = source[i];
    i++;
    if (n >= 0xC0) {
      switch (n) {
        case 0xD0:
          {
            n = source[i];
            i++;
            if (n == 0x81) {
              n = 0xA8;
              break;
            }
            if (n >= 0x90 && n <= 0xBF) n += 0x30;
            break;
          }
        case 0xD1:
          {
            n = source[i];
            i++;
            if (n == 0x91) {
              n = 0xB8;
              break;
            }
            if (n >= 0x80 && n <= 0x8F) n += 0x70;
            break;
          }
      }
    }
    m[0] = n;
    target += m;
  }
  return target;
}

void drawStringMax(String tape, int start) {
  memset(data, '\0', LED_MAX_BUF);
  utf2rus(tape).toCharArray(data, LED_MAX_BUF);
  newMessageAvailable = true;
}

#define SCROLL_SPEED 50
#define PAUSE_TIME 1000

void realDisplayText() {
  static char dataTmp[LED_MAX_BUF]; // Use static to maintain the buffer state between calls

  if (M.displayAnimate()) {
    if (newMessageAvailable) {
      newMessageAvailable = false;
      memset(dataTmp, '\0', LED_MAX_BUF);
      strcpy(dataTmp, data);
      M.displayReset();
      Serial.println(dataTmp);

      M.displayClear(); // Clear the display before setting new text

      if (strlen(dataTmp) > 5) {
        M.displayText(dataTmp, PA_LEFT, SCROLL_SPEED, PAUSE_TIME, PA_SCROLL_LEFT, PA_NO_EFFECT);
      } else {
        M.displayText(dataTmp, PA_CENTER, SCROLL_SPEED, PAUSE_TIME, PA_PRINT, PA_NO_EFFECT);
      }
    }
  }
}

bool displayAnimate()
{
  return M.displayAnimate();
}

void matrixSetup() {
  M.begin();
  M.displayClear();
  M.displaySuspend(false);
  M.setInvert(false);
  M.setFont(CRMrusTxt);
  M.setIntensity(0);
}

void printText(String text) {
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