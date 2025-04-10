#include "OTAUpdate.h"

// Global variable definition
String pathOta;

// OTA callbacks
void update_started() {
  if (Serial) Serial.println(F("CALLBACK: HTTP update process starteds"));
  printText(F("Update"));
}

void update_finished() {
  if (Serial) Serial.println(F("CALLBACK: HTTP update process finished"));
  printText(F("Restart"));
}

void update_progress(int cur, int total) {
  printText(String((cur / (total / 100)), DEC) + " %");
  if (Serial) Serial.printf("CALLBACK: HTTP update process at %d of %d bytes...\n", cur, total);
}

void update_error(int err) {
  Serial.printf("CALLBACK: HTTP update fatal error code %d\n", err);
}

// OTA initialization
void web_ota_init() {
    ESPhttpUpdate.onStart(update_started);
    ESPhttpUpdate.onEnd(update_finished);
    ESPhttpUpdate.onProgress(update_progress);
    ESPhttpUpdate.onError(update_error);
    ESPhttpUpdate.setClientTimeout(2000);
    
    // Constructing OTA URL
    pathOta = webOTA_updateURL + F("?MAC=") + macAddrSt + F("&hst=") + hostname_m + F("&ip=") + ip + F("&ver=") + version_prg;
}

// Perform OTA update
void update_ota() {
    //detachInterrupt_clock_process();

    BearSSL::WiFiClientSecure client;
    client.setInsecure();

    if (Serial) Serial.println(F("Calling: ") + pathOta);

    // Perform the update and check the result
    t_httpUpdate_return ret = ESPhttpUpdate.update(client, pathOta, version_prg);

    if (Serial) Serial.printf("Returned code %d\n", ret);

    // Handle update result
    switch (ret) {
      case HTTP_UPDATE_FAILED:
        if (Serial) Serial.printf("HTTP_UPDATE_FAILED Error (%d): %s\n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
        break;

      case HTTP_UPDATE_NO_UPDATES:
        if (Serial) Serial.println(F("HTTP_UPDATE_NO_UPDATES"));
        break;

      case HTTP_UPDATE_OK:
        if (Serial) Serial.println(F("HTTP_UPDATE_OK"));
        break;
    }

    // init_clock_process();
}
