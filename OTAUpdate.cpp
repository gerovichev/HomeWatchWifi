#include "OTAUpdate.h"
#include "constants.h"
#include "logger.h"
#include "location_manager.h"
#include <WiFiClientSecureBearSSL.h>

// Global variable definition
String pathOta;

// Root CA for gerovichomv.duckdns.org (the OTA update host). Its certificate
// is issued by Let's Encrypt and currently chains up to this self-signed
// ISRG root (valid until 2035), so pinning it here survives normal
// leaf/intermediate renewal.
static const char OTA_ROOT_CA[] PROGMEM = R"CERT(
-----BEGIN CERTIFICATE-----
MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw
TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh
cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4
WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu
ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY
MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc
h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+
0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U
A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW
T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH
B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC
B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv
KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn
OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn
jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw
qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI
rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV
HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq
hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL
ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ
3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK
NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5
ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur
TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC
jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc
oyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq
4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA
mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d
emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=
-----END CERTIFICATE-----
)CERT";

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
  BearSSL::X509List rootCA(OTA_ROOT_CA);
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
