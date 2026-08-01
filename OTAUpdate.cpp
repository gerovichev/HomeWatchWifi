#include "OTAUpdate.h"
#include "constants.h"
#include "logger.h"
#include "location_manager.h"
#include "root_certs.h"
#include <WiFiClientSecureBearSSL.h>

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
  int percent = (total > 0) ? ((cur * 100) / total) : 0;
  char buf[8];
  snprintf(buf, sizeof(buf), "%d %%", percent);
  printText(String(buf));
  LOG_VERBOSE_FMT("OTA progress: %d/%d bytes (%d%%)", cur, total, percent);
}

void update_error(int err) {
  LOG_ERROR_FMT("OTA update fatal error code: %d (%s)",
                err, ESPhttpUpdate.getLastErrorString().c_str());
}

// OTA initialization
void web_ota_init() {
  ESPhttpUpdate.onStart(update_started);
  ESPhttpUpdate.onEnd(update_finished);
  ESPhttpUpdate.onProgress(update_progress);
  ESPhttpUpdate.onError(update_error);
  ESPhttpUpdate.setClientTimeout(Timing::OTA_CLIENT_TIMEOUT_MS);

  // Build OTA URL into a pre-sized buffer — avoids repeated heap allocations
  // from chaining F() string fragments.
  pathOta.reserve(strlen(webOTA_updateURL) + macAddrSt.length() +
                  hostname_m.length() + ip.length() + version_prg.length() + 24);
  char pathBuf[Buffer::PATH_BUFFER_SIZE];
  snprintf(pathBuf, sizeof(pathBuf), "%s?MAC=%s&hst=%s&ip=%s&ver=%s",
           webOTA_updateURL, macAddrSt.c_str(), hostname_m.c_str(),
           ip.c_str(), version_prg.c_str());
  pathOta = pathBuf;
}

// Perform OTA update
void update_ota() {
  // BearSSL needs a real system clock to validate the certificate's
  // notBefore/notAfter dates; this is a no-op once NTP has already synced.
  setClock();

  BearSSL::WiFiClientSecure client;
  BearSSL::X509List rootCA(ISRG_ROOT_X1);
  client.setTrustAnchors(&rootCA);

  LOG_DEBUG_FMT("OTA URL: %s", pathOta.c_str());

  // Perform the update and check the result
  t_httpUpdate_return ret = ESPhttpUpdate.update(client, pathOta, version_prg);

  LOG_DEBUG_FMT("OTA returned code: %d", (int)ret);

  // Handle update result
  switch (ret) {
  case HTTP_UPDATE_FAILED:
    LOG_ERROR_FMT("OTA update failed: Error %d - %s",
                  ESPhttpUpdate.getLastError(),
                  ESPhttpUpdate.getLastErrorString().c_str());
    break;

  case HTTP_UPDATE_NO_UPDATES:
    LOG_INFO_F("No OTA updates available");
    break;

  case HTTP_UPDATE_OK:
    LOG_INFO_F("OTA update completed successfully");
    break;
  }
}
