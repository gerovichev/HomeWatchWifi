#include "OTAUpdate.h"
#include "logger.h"
#include "secure_client.h"
#include "constants.h"

// Global variable definition
String pathOta;

// OTA callbacks
void update_started() {
  LOG_INFO_F("OTA update process started");
  printText(F("Update"));
}

void update_finished() {
  LOG_INFO_F("OTA update process finished");
  printText(F("Restart"));
}

void update_progress(int cur, int total) {
  int percent = cur / (total / 100);
  printText(String(percent, DEC) + " %");
  LOG_VERBOSE("OTA progress: " + String(cur) + "/" + String(total) + " bytes (" + String(percent) + "%)");
}

void update_error(int err) {
  LOG_ERROR("OTA update fatal error code: " + String(err));
}

// OTA initialization
void web_ota_init() {
    ESPhttpUpdate.onStart(update_started);
    ESPhttpUpdate.onEnd(update_finished);
    ESPhttpUpdate.onProgress(update_progress);
    ESPhttpUpdate.onError(update_error);
    ESPhttpUpdate.setClientTimeout(Timing::OTA_CLIENT_TIMEOUT_MS);
    
    // Constructing OTA URL - optimize to reduce String allocations
    pathOta.reserve(strlen(webOTA_updateURL) + macAddrSt.length() + hostname_m.length() + ip.length() + version_prg.length() + 50);
    pathOta = String(webOTA_updateURL) + F("?MAC=") + macAddrSt + F("&hst=") + hostname_m + F("&ip=") + ip + F("&ver=") + version_prg;
}

// Perform OTA update
void update_ota() {
    BearSSL::WiFiClientSecure client;
    setupSecureClient(client, "OTA server");

    LOG_DEBUG("OTA URL: " + pathOta);

    // Perform the update and check the result
    t_httpUpdate_return ret = ESPhttpUpdate.update(client, pathOta, version_prg);

    LOG_DEBUG("OTA returned code: " + String(ret));

    // Handle update result
    switch (ret) {
      case HTTP_UPDATE_FAILED:
        LOG_ERROR("OTA update failed: Error " + String(ESPhttpUpdate.getLastError()) + 
                  " - " + ESPhttpUpdate.getLastErrorString());
        break;

      case HTTP_UPDATE_NO_UPDATES:
        LOG_INFO_F("No OTA updates available");
        break;

      case HTTP_UPDATE_OK:
        LOG_INFO_F("OTA update completed successfully");
        break;
    }
}
