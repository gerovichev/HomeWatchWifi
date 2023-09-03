#pragma once

#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>


#include "clock_process.h"


ESP8266WiFiMulti WiFiMulti;

void update_started() {
  Serial.println("CALLBACK:  HTTP update process started");
  drawString("Update", 0);
}

void update_finished() {
  Serial.println("CALLBACK:  HTTP update process finished");
  drawString("Restart", 0);
}

void update_progress(int cur, int total) {
  drawString(String((cur / (total / 100)), DEC) + " %", 0);
  Serial.printf("CALLBACK:  HTTP update process at %d of %d bytes...\n", cur, total);
}

void update_error(int err) {
  //drawString("Error updated", 0);
  Serial.printf("CALLBACK:  HTTP update fatal error code %d\n", err);
}


void update_ota() {

  detachInterrupt_clock_process();
  // wait for WiFi connection
  if ((WiFiMulti.run() == WL_CONNECTED)) {

    BearSSL::WiFiClientSecure client;
    client.setInsecure();

    // The line below is optional. It can be used to blink the LED on the board during flashing
    // The LED will be on during download of one buffer of data from the network. The LED will
    // be off during writing that buffer to flash
    // On a good connection the LED should flash regularly. On a bad connection the LED will be
    // on much longer than it will be off. Other pins than LED_BUILTIN may be used. The second
    // value is used to put the LED on. If the LED is on with HIGH, that value should be passed
    //ESPhttpUpdate.setLedPin(LED_BUILTIN, LOW);

    // Add optional callback notifiers
    ESPhttpUpdate.onStart(update_started);
    ESPhttpUpdate.onEnd(update_finished);
    ESPhttpUpdate.onProgress(update_progress);
    ESPhttpUpdate.onError(update_error);

    String path = webOTA_updateURL + "?MAC=" + macAddrSt + "&hst=" + hostname_m + "&ip=" + ip + "&ver=" + version_prg;
    
    Serial.println("Calling: " + path);

    t_httpUpdate_return ret = ESPhttpUpdate.update(client, path);

    Serial.printf("returned code %d\n", ret);

    switch (ret) {
      case HTTP_UPDATE_FAILED:
        Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s\n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
        break;

      case HTTP_UPDATE_NO_UPDATES:
        Serial.println("HTTP_UPDATE_NO_UPDATES");
        break;

      case HTTP_UPDATE_OK:
        Serial.println("HTTP_UPDATE_OK");
        break;
    }
  }

  init_clock_process();
}
