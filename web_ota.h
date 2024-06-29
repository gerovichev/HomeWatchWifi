#pragma once

#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include "clock_process.h"


void update_started() {
  Serial.println("CALLBACK: HTTP update process started");
  printText("Update");
}

void update_finished() {
  Serial.println("CALLBACK: HTTP update process finished");
  printText("Restart");
}

void update_progress(int cur, int total) {
  printText(String((cur / (total / 100)), DEC) + " %");
  Serial.printf("CALLBACK: HTTP update process at %d of %d bytes...\n", cur, total);
}

void update_error(int err) {
  Serial.printf("CALLBACK: HTTP update fatal error code %d\n", err);
}

void web_ota_init(){
    ESPhttpUpdate.onStart(update_started);
    ESPhttpUpdate.onEnd(update_finished);
    ESPhttpUpdate.onProgress(update_progress);
    ESPhttpUpdate.onError(update_error);
    ESPhttpUpdate.setClientTimeout(2000);
}

void update_ota() {
  detachInterrupt_clock_process();

    BearSSL::WiFiClientSecure client;
    client.setInsecure();

    String path = webOTA_updateURL + "?MAC=" + macAddrSt + "&hst=" + hostname_m + "&ip=" + ip + "&ver=" + version_prg;
    Serial.println("Calling: " + path);

    t_httpUpdate_return ret = ESPhttpUpdate.update(client, path, version_prg);

    Serial.printf("Returned code %d\n", ret);

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

  init_clock_process();
}
