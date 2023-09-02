#pragma once

#include <Adafruit_GFX.h>
#include <SPI.h>
#include <Max72xxPanel.h>
//#include <Fonts/FreeMonoOblique9pt7b.h>

void setIntensity(byte intensity);

#include "global.h"
//#include "livolo_ha.h"

#define LIGHT_SENSOR A0

int pinCS = 5;
int numberOfHorizontalDisplays = 1;
int numberOfVerticalDisplays = 4;

Max72xxPanel matrix = Max72xxPanel(pinCS, numberOfHorizontalDisplays, numberOfVerticalDisplays);

String utf8rus(String source) {
  int i, k;
  String target;
  unsigned char n;
  char m[2] = { '0', '\0' };

  k = source.length();
  i = 0;

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
            if (n >= 0x90 && n <= 0xBF) n = n + 0x30;
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
            if (n >= 0x80 && n <= 0x8F) n = n + 0x70;
            break;
          }
      }
    }
    m[0] = n;
    target = target + String(m);
  }
  return target;
}

String prevTape = "";

unsigned wait = 50;  // скорость бегущей строки
unsigned spacer = 1;
unsigned width = 5 + spacer;  // Регулируем расстояние между символами
int refresh = 0;
// =======================================================================
void scrollText(String text) {
  for (int i = 0; i < width * text.length() - 1; i++) {
    /*if (isRadioSigAvailable()) {
      break;
    }*/
    if (refresh == 1) i = 0;
    refresh = 0;
    matrix.fillScreen(LOW);
    int letter = i / width;
    int x = (matrix.width() - 1) - i % width;
    int y = (matrix.height() - 8) / 2;  // Центрируем текст по Вертикали
    while (x + width - spacer >= 0 && letter >= 0) {
      /* if (isRadioSigAvailable()) {
        break;
      }*/
      if (letter < text.length()) {
        matrix.drawChar(x, y, text[letter], HIGH, LOW, 1);
        Serial.print(text[letter]);
      }
      letter--;
      x -= width;
    }
    matrix.write();  // Вывод на дисплей
    Serial.println();
    delay(wait);
    yield();
  }
}
// =======================================================================


void drawStringWithDelay(String tape, int start, boolean isDelay) {


  //String forprint = tape;
  if (lang_weather.compareTo("en") != 0) {
    //Serial.println("compare: " + String(lang_weather.compareTo("en"), DEC));
    tape = utf8rus(tape);
  }

  if (!prevTape.equals(tape)) {

    //Serial.println("length: " + String(tape.length(), DEC));
    if (tape.length() <= 5 || !isDelay) {
      matrix.fillScreen(LOW);
      for (int i = 0; i < tape.length() * 2; i++) {
        matrix.drawChar((i + start) * 6 + 1, 0, tape[i], HIGH, LOW, 1);
        Serial.print(tape[i]);
      }
      matrix.write();
      Serial.println();
    } else {

      scrollText(tape);
      /*for (int i = 0; i < tape.length()/2 - 4; i++) {
        matrix.fillScreen(LOW);
        Serial.println(tape.length());
        for (int j = 0; j < tape.length() - 1; j++) {
          matrix.drawChar((j + start) * 6 + 1 , 0, tape[i + j], HIGH, LOW, 1);
          Serial.print(forprint[i + j]);
        }
        matrix.write();
        Serial.println();
        if (isDelay && !mySwitch.available()) {
          delay(1000);
        }

        }*/
    }

    //Serial.println(tape);
    prevTape = tape;
  }
}

void drawStringMax(String tape, int start) {
  drawStringWithDelay(tape, start, true);
}

void setIntensity(byte intensity) {
  matrix.setIntensity(intensity);
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


  matrix.setIntensity(intensity);
}

void matrixSetup() {
  matrix.setRotation(3);
  setIntensity(0);
  matrix.cp437(true);
  //matrix.setFont(&FreeMonoOblique9pt7b); // выбор шрифта
}
