#include "led_display.h"
#include "constants.h"
#include "logger.h"

// Global variables
bool newMessageAvailable = false;
MD_Parola M = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);
String lastDisplayedText = "";

// LEDBuffer class implementation
LEDBuffer::LEDBuffer(size_t size) : bufferSize(size) {
  buffer.resize(bufferSize);
  clearBuffer();
}

void LEDBuffer::clearBuffer() { std::fill(buffer.begin(), buffer.end(), '\0'); }

char *LEDBuffer::getBuffer() { return buffer.data(); }

size_t LEDBuffer::getBufferSize() const { return bufferSize; }

// Sets the intensity of the display
void setIntensity(byte intensity) { M.setIntensity(intensity); }

// Function to convert time_t to a readable string format
String formatTime(time_t rawTime) {
  int hours = (rawTime % 86400L) / 3600;
  int minutes = (rawTime % 3600) / 60;
  int seconds = rawTime % 60;

  char timeStr[Buffer::TIME_STRING_SIZE];
  sprintf(timeStr, "%02d:%02d:%02d", hours, minutes, seconds);

  return String(timeStr);
}

// Sets the intensity based on the current time
void setIntensityByTime(time_t timeNow) {
  int intensity = (timeNow > sunrise && timeNow < sunset)
                      ? Display::INTENSITY_DAY
                      : Display::INTENSITY_NIGHT;

  LOG_VERBOSE("sunrise: " + formatTime(sunrise));
  LOG_VERBOSE("Time: " + formatTime(timeNow));
  LOG_VERBOSE("sunset: " + formatTime(sunset));
  LOG_VERBOSE("intensity: " + String(intensity));
  M.setIntensity(intensity);
}

// Converts UTF-8 to Russian characters
String utf2rus(const String &source) {
  String target;
  target.reserve(source.length());

  for (int i = 0, k = source.length(); i < k; ++i) {
    unsigned char n = source[i];
    if (n >= 0xC0) {
      switch (n) {
      case 0xD0: {
        n = source[++i];
        if (n == 0x81) {
          n = 0xA8;
        } else if (n >= 0x90 && n <= 0xBF) {
          n += 0x30;
        }
        break;
      }
      case 0xD1: {
        n = source[++i];
        if (n == 0x91) {
          n = 0xB8;
        } else if (n >= 0x80 && n <= 0x8F) {
          n += 0x70;
        }
        break;
      }
      }
    }
    target += char(n);
  }

  return target;
}

// Draws a string on the LED display
void drawStringMax(const String &tape) {
  String convertedText = utf2rus(tape);
  if (convertedText != lastDisplayedText) {
    lastDisplayedText = convertedText;
    newMessageAvailable = true;
  }
}

// Displays the text on the LED display
void realDisplayText() {
  if (M.displayAnimate() && newMessageAvailable) {
    newMessageAvailable = false;
    M.displayReset();
    LOG_INFO(">> Display: " + lastDisplayedText);
    M.displayClear();

    if (lastDisplayedText.length() > 5) {
      M.displayText(lastDisplayedText.c_str(), PA_LEFT,
                    Display::SCROLL_SPEED_MS, Display::PAUSE_TIME_MS,
                    PA_SCROLL_LEFT, PA_NO_EFFECT);
    } else {
      M.displayText(lastDisplayedText.c_str(), PA_CENTER,
                    Display::SCROLL_SPEED_MS, Display::PAUSE_TIME_MS, PA_PRINT,
                    PA_NO_EFFECT);
    }
  }
}

// Force display text (for use in setup when display may not be ready)
void forceDisplayText() {
  if (newMessageAvailable) {
    newMessageAvailable = false;
    M.displayReset();
    LOG_INFO(">> Force Display: " + lastDisplayedText);
    M.displayClear();

    if (lastDisplayedText.length() > 5) {
      M.displayText(lastDisplayedText.c_str(), PA_LEFT,
                    Display::SCROLL_SPEED_MS, Display::PAUSE_TIME_MS,
                    PA_SCROLL_LEFT, PA_NO_EFFECT);
    } else {
      M.displayText(lastDisplayedText.c_str(), PA_CENTER,
                    Display::SCROLL_SPEED_MS, Display::PAUSE_TIME_MS, PA_PRINT,
                    PA_NO_EFFECT);
    }
  }
}

// Wait for animation to complete
void waitForAnimation() {
  while (!displayAnimate()) {
    delay(50);
  }
}

// Display text in setup (draw, force display, and wait for animation)
void displayTextInSetup(const String &text) {
  drawStringMax(text);
  forceDisplayText();
  waitForAnimation();
}

// Animates the display
bool displayAnimate() { return M.displayAnimate(); }

// Initializes the LED matrix display
void matrixSetup() {
  M.begin();
  M.displayClear();
  M.displaySuspend(false);
  M.setInvert(false);
  M.setFont(CRMrusTxt);
  M.setIntensity(Display::INTENSITY_NIGHT); // Set visible intensity for startup

  // Small delay to ensure display is ready
  delay(100);

  // Display "MAX7201" immediately at startup with points
  M.displayReset();
  M.displayClear();
  // M.displayText("MAX7201", PA_CENTER, 0, 0, PA_PRINT, PA_NO_EFFECT);

  // Allow display to stabilize and show the text
  delay(200);

  // Process display animation to ensure text is shown
  for (int i = 0; i < 10; i++) {
    M.displayAnimate();
    delay(50);
  }
}

// Prints the given text on the LED display
void printText(String text) {
  LOG_DEBUG("Printing text: " + text);
  char dataText[Buffer::LED_BUFFER_SIZE];
  utf2rus("     " + text).toCharArray(dataText, Buffer::LED_BUFFER_SIZE);

  int textLength = strlen(dataText) - 6;

  if (textLength > 5) {
    for (int i = 0; i < textLength; i++) {
      M.print(&dataText[i]);
      // Use yield() instead of delay() to allow other tasks to run
      // Small delay is acceptable here for display animation
      yield();
      delay(250); // Keep delay for display timing, but allow yield
    }
  } else {
    M.print(&dataText[5]);
  }
}
