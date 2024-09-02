#pragma once

#include <ESP8266WebServer.h>
#include <ESP8266SSDP.h>
#include "web_ota.h" // Include the web OTA header file
//#include "WiFiManagerWrapper.h"

ESP8266WebServer webServer(80);

void wifiReset() {

  Serial.println("Web reading");

  String message = "";
  int code = HTTP_CODE_OK;

  wifi_init();
  //wifiManagerWrapper.wifi_reset();
  // Prepare the response
  message = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>\r\nWifi reset \r\n";
  message += "</html>\n";

  webServer.sendHeader("Connection", "close");
  webServer.send(code, "text/html", message);  //Returns the HTTP response

  ESP.restart();
}


void handleRestart() {
  Serial.println("Restarting...");

  // Prepare the response HTML
  String message = "<!DOCTYPE HTML>\r\n<html>\r\n<head>\r\n";
  message += "<meta http-equiv='refresh' content='5;url=/' />\r\n";
  message += "<title>Restarting</title>\r\n";
  message += "<style>\r\n";
  message += "body { background-color: #f0f0f0; font-family: Arial, Helvetica, Sans-Serif; color: #333; text-align: center; padding-top: 50px; }\r\n";
  message += ".container { background-color: #fff; border-radius: 8px; box-shadow: 0 0 10px rgba(0, 0, 0, 0.1); padding: 20px; max-width: 500px; margin: 0 auto; }\r\n";
  message += "</style>\r\n";
  message += "</head>\r\n<body>\r\n";
  message += "<div class='container'>\r\n<h1>Restarting</h1>\r\n";
  message += "<p>Your device is restarting. The device will reboot in a few seconds.</p>\r\n";
  message += "<p>If it doesn't redirect automatically, <a href='/'>click here</a>.</p>\r\n";
  message += "</div>\r\n</body>\r\n</html>";

  // Send the HTML response
  webServer.sendHeader("Connection", "close");
  webServer.send(HTTP_CODE_OK, "text/html", message);  //Returns the HTTP response

  // Add a delay to allow the message to be displayed before restarting
  delay(5000);

  // Restart the ESP8266
  ESP.restart();
}

const char htmlTemplate[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta http-equiv='refresh' content='5'/>
  <meta name='viewport' content='width=device-width, initial-scale=1.0'/>
  <title>ESP8266 Demo</title>
  <style>
    body {
      background-color: #f0f0f0;
      font-family: Arial, Helvetica, Sans-Serif;
      color: #333;
      margin: 0;
      padding: 0;
      display: flex;
      justify-content: center;
      align-items: center;
      height: 100vh;
    }
    .container {
      background-color: #fff;
      border-radius: 8px;
      box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);
      padding: 20px;
      max-width: 500px;
      width: 100%;
      text-align: center;
    }
    h1 {
      color: #007BFF;
    }
    p {
      margin: 10px 0;
    }
    a {
      color: #007BFF;
      text-decoration: none;
      margin: 0 5px;
    }
    a:hover {
      text-decoration: underline;
    }
    button {
      margin-top: 20px;
      padding: 10px 20px;
      background-color: #007BFF;
      color: #fff;
      border: none;
      border-radius: 5px;
      cursor: pointer;
      font-size: 16px;
    }
    button:hover {
      background-color: #0056b3;
    }
    @media (max-width: 600px) {
      body {
        padding: 10px;
      }
      .container {
        padding: 15px;
      }
    }
  </style>
  <script>
    function goBack() {
      window.history.back();
    }
  </script>
</head>
<body>
  <div class='container'>
    <h1>Hello from ESP8266!</h1>
    <p>Current time: %s</p>
    <button onclick='goBack()'>Back</button>
  </div>
</body>
</html>
)rawliteral";

void handleTime() {
  // Get the current formatted time
  String currentTime = timeClient.getFormattedTime();

  // Calculate the size of the buffer required
  size_t htmlSize = strlen(htmlTemplate) + currentTime.length() - 2 + 1; // -2 for the %s and +1 for null-terminator
  char html[htmlSize];

  // Format the HTML with the current time
  int bytesWritten = snprintf(html, htmlSize, htmlTemplate, currentTime.c_str());

  // Check for potential issues
  if (bytesWritten < 0 || (size_t)bytesWritten >= htmlSize) {
      // Handle the error or truncation case
      webServer.send(500, "text/plain", "Internal Server Error");
      return;
  }

  // Send the HTML response
  webServer.sendHeader("Connection", "close");
  webServer.send(200, "text/html", html);
}

const char htmlPage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html xmlns='http://www.w3.org/1999/xhtml'>
<head>
    <meta http-equiv='refresh' content='5'/>
    <meta name='viewport' content='width=device-width, initial-scale=1.0'/>
    <title>ESP8266 Demo</title>
    <style>
        body {
            background-color: #f0f0f0;
            font-family: Arial, Helvetica, Sans-Serif;
            color: #333;
            margin: 0;
            padding: 0;
            display: flex;
            justify-content: center;
            align-items: center;
            height: 100vh;
        }
        .container {
            background-color: #fff;
            border-radius: 8px;
            box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);
            padding: 20px;
            max-width: 500px;
            width: 100%;
            text-align: center;
        }
        h1 {
            color: #007BFF;
        }
        p {
            margin: 10px 0;
        }
        a {
            color: #007BFF;
            text-decoration: none;
            margin: 0 5px;
        }
        a:hover {
            text-decoration: underline;
        }
        @media (max-width: 600px) {
            body {
                padding: 10px;
            }
            .container {
                padding: 15px;
            }
        }
    </style>
</head>
<body>
    <div class='container'>
        <h1>Hello from ESP8266! %s - %s</h1>
        <p>Uptime: %02lu days %02lu:%02lu:%02lu</p>
        <p><a href='/time'>Time</a></p>
        <p><a href='/restart'>Restart</a></p>
        <p><a href='/ota-update'>OTA</a></p>
        <p><a href='/wifireset'>WiFi Reset</a></p>
    </div>
</body>
</html>
)rawliteral";

unsigned long prevMillis = 0;
unsigned long totalMillis = 0;

void updateTotalMillis() {
    unsigned long currentMillis = millis();
    
    if (currentMillis < prevMillis) {
        // Overflow detected
        totalMillis += (4294967295 - prevMillis) + currentMillis + 1;
    } else {
        totalMillis += (currentMillis - prevMillis);
    }

    prevMillis = currentMillis;
}

void handleRoot() {

  size_t baseSize = strlen(htmlPage);
  size_t dynamicSize = hostname_m.length() + version_prg.length();
  size_t timeSize = 4 * 10; // Maximum possible size for days, hours, minutes, seconds (10 digits each)
  size_t totalSize = baseSize + dynamicSize + timeSize + 20; // Adding a margin

  char temp[totalSize]; // Allocate the buffer with the calculated size

  // Calculate and display the working time
  updateTotalMillis();
  unsigned long workingTimeMillis = totalMillis;
  unsigned long seconds = (workingTimeMillis / 1000) % 60;
  unsigned long minutes = (workingTimeMillis / (1000 * 60)) % 60;
  unsigned long hours = (workingTimeMillis / (1000 * 60 * 60)) % 24;
  unsigned long days = (workingTimeMillis / (1000 * 60 * 60 * 24));


  int bytesWritten = snprintf(temp, sizeof(temp), htmlPage, hostname_m.c_str(), version_prg.c_str(), days, hours, minutes, seconds);
      // Check if snprintf caused truncation
    if (bytesWritten < 0 || (size_t)bytesWritten >= totalSize) {
        // Handle the truncation or error case
        webServer.send(500, "text/plain", "Internal Server Error: Content too long");
        return;
    }
  webServer.sendHeader("Connection", "close");
  webServer.send(HTTP_CODE_OK, "text/html", temp);
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += webServer.uri();
  message += "\nMethod: ";
  message += (webServer.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += webServer.args();
  message += "\n";

  for (uint8_t i = 0; i < webServer.args(); i++) {
    message += " " + webServer.argName(i) + ": " + webServer.arg(i) + "\n";
  }

  webServer.sendHeader("Connection", "close");
  webServer.send(HTTP_CODE_NOT_FOUND, "text/plain", message);
}

// Function for initiating the web OTA update
void handleWebOtaUpdate() {
  update_ota(); // Call the existing update_ota function from web_ota.h
  webServer.send(200, "text/plain", "OTA update triggered"); // Send a response to indicate the OTA update was triggered
}

void ssdp_init() {
  Serial.println("Starting HTTP...");
  webServer.on("/index.html", HTTP_GET, []() {
    webServer.sendHeader("Connection", "close");
    webServer.send(200, "text/plain", "Hello World!");
  });

  //webServer.begin();

  Serial.println(F("Starting SSDP..."));
  SSDP.setSchemaURL(F("description.xml"));
  SSDP.setHTTPPort(80);
  SSDP.setName(F("ESP watch"));
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


void webserver_init() {
  if (isWebClientNeeded) {

    ssdp_init();

    webServer.on("/time", handleTime);
    webServer.on("/restart", handleRestart);
    webServer.on("/ota-update", HTTP_GET, handleWebOtaUpdate); // Define a route for triggering the OTA update
    webServer.on("/wifireset", wifiReset);
    webServer.on("/", handleRoot);
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
