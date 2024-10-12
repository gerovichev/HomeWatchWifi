#include "web_server.h"
#include "WiFiSetup.h"
#include <ESP8266SSDP.h>

//#include "web_ota.h"  // Include the web OTA header file
//#include "time_client.h"  // Assuming timeClient is defined in another file

ESP8266WebServer webServer(80);
unsigned long prevMillis = 0;
unsigned long totalMillis = 0;

// Common HTML template stored in PROGMEM
const char htmlTemplate[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta http-equiv='refresh' content='%d'/>
  <meta name='viewport' content='width=device-width, initial-scale=1.0'/>
  <title>%s</title>
  <style>
    body { background-color: #f0f0f0; font-family: Arial, Helvetica, Sans-Serif; color: #333; margin: 0; padding: 0; display: flex; justify-content: center; align-items: center; height: 100vh; }
    .container { background-color: #fff; border-radius: 8px; box-shadow: 0 0 10px rgba(0, 0, 0, 0.1); padding: 20px; max-width: 500px; width: 100%; text-align: center; }
    h1 { color: #007BFF; }
    p { margin: 10px 0; }
    a { color: #007BFF; text-decoration: none; margin: 0 5px; }
    a:hover { text-decoration: underline; }
    button { margin-top: 20px; padding: 10px 20px; background-color: #007BFF; color: #fff; border: none; border-radius: 5px; cursor: pointer; font-size: 16px; }
    button:hover { background-color: #0056b3; }
  </style>
  <script>function goBack() { window.history.back(); }</script>
</head>
<body>
  <div class='container'>
    <h1>%s</h1>
    <p>%s</p>
    <div>
        <p><a href='/time'>Time</a></p>
        <p><a href='/restart'>Restart</a></p>
        <p><a href='/ota-update'>OTA</a></p>
        <p><a href='/wifireset'>WiFi Reset</a></p>
    </div>
    <button onclick='goBack()'>Back</button>
  </div>
</body>
</html>
)rawliteral";

// Function to send dynamic HTML response
void sendDynamicHTMLResponse(const char* title, const char* message, int refreshTime) {
  char html[sizeof(htmlTemplate) + sizeof(title) * 2 + sizeof(message) + 100];
  snprintf(html, sizeof(html), htmlTemplate, refreshTime, title, title, message);

  webServer.sendHeader("Connection", "close");
  webServer.send(HTTP_CODE_OK, "text/html", html);
}

// WiFi reset handler
void wifiReset() {
  if (Serial) Serial.println(F("WiFi Resetting..."));
  sendDynamicHTMLResponse("WiFi Reset", "The WiFi is being reset.", 3600);
  WIFISetup wifiSetup;
  wifiSetup.wifi_reset();
  ESP.restart();
}

// Restart handler
void handleRestart() {
  if (Serial) Serial.println(F("Restarting..."));
  sendDynamicHTMLResponse("Restarting", "Your device is restarting. It will reboot shortly.", 3600);
  delay(5000);
  ESP.restart();
}

// Time handler
void handleTime() {
  String currentTime = timeClient.getFormattedTime();
  sendDynamicHTMLResponse("Current Time", ("The current time is: " + currentTime).c_str(), 5);
}

// Update total milliseconds
void updateTotalMillis() {
  unsigned long currentMillis = millis();
  if (currentMillis < prevMillis) {
    totalMillis += (4294967295 - prevMillis) + currentMillis + 1;
  } else {
    totalMillis += (currentMillis - prevMillis);
  }
  prevMillis = currentMillis;
}

// Root handler with uptime information
void handleRoot() {
  updateTotalMillis();
  unsigned long uptimeSec = totalMillis / 1000;
  unsigned long seconds = uptimeSec % 60;
  unsigned long minutes = (uptimeSec / 60) % 60;
  unsigned long hours = (uptimeSec / 3600) % 24;
  unsigned long days = uptimeSec / 86400;

  char uptimeStr[64];
  snprintf(uptimeStr, sizeof(uptimeStr), "Uptime: %lu days, %02lu:%02lu:%02lu", days, hours, minutes, seconds);
  char headerStr[64]; 
  snprintf(headerStr, sizeof(headerStr), "Hello from ESP8266!<br/>%s - %s", hostname_m.c_str(), version_prg.c_str());
  
  sendDynamicHTMLResponse(headerStr, uptimeStr, 5);
}

// 404 Not Found handler
void handleNotFound() {
  String message = F("File Not Found\n\nURI: ");
  message += webServer.uri();
  message += F("\nMethod: ");
  message += (webServer.method() == HTTP_GET) ? "GET" : "POST";
  message += F("\nArguments: ");
  message += webServer.args();
  message += F("\n");

  for (uint8_t i = 0; i < webServer.args(); i++) {
    message += " " + webServer.argName(i) + ": " + webServer.arg(i) + "\n";
  }

  webServer.sendHeader("Connection", "close");
  webServer.send(HTTP_CODE_NOT_FOUND, "text/plain", message);
}

// OTA Update Handler
void handleWebOtaUpdate() {
  update_ota();  // Trigger OTA update
  webServer.send(HTTP_CODE_OK, F("text/plain"), F("OTA update triggered"));
}

// SSDP initialization
void ssdp_init() {
  if (Serial) Serial.println(F("Starting SSDP..."));
  SSDP.setSchemaURL(F("description.xml"));
  SSDP.setHTTPPort(80);
  SSDP.setName(F("ESP Watch"));
  SSDP.setSerialNumber(F("000000001"));
  SSDP.setURL(F("/"));
  SSDP.setModelName(F("ESP Watch 2024"));
  SSDP.setModelNumber(F("01"));
  SSDP.setModelURL(F("https://gerovichev.wixsite.com/gerovichev"));
  SSDP.setManufacturer(F("Gerovichev Electronics"));
  SSDP.setManufacturerURL(F("https://gerovichev.wixsite.com/gerovichev"));
  SSDP.begin();

  webServer.on(F("/description.xml"), HTTP_GET, []() {
    SSDP.schema(webServer.client());
  });
}

// Web server initialization
void webserver_init() {
  if (isWebClientNeeded) {
    ssdp_init();
    webServer.on(F("/time"), handleTime);
    webServer.on(F("/restart"), handleRestart);
    webServer.on(F("/ota-update"), HTTP_GET, handleWebOtaUpdate);
    webServer.on(F("/wifireset"), wifiReset);
    webServer.on(F("/"), handleRoot);
    webServer.onNotFound(handleNotFound);
    webServer.keepAlive(true);
    webServer.begin();
  }
}

void webClientHandle() {
  if (isWebClientNeeded) {
    webServer.handleClient();
  }
}
