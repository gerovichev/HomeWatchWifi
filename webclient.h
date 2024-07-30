#pragma once

#include <ESP8266SSDP.h>
#include "web_ota.h" // Include the web OTA header file
//#include "WiFiManagerWrapper.h"

ESP8266WebServer webServer(80);

const int RED = 15;
const int GREEN = 12;
const int BLUE = 13;

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

void handleLed() {
  String message = "";
  int code = HTTP_CODE_OK;
  if (!webServer.hasArg("color") || !webServer.hasArg("val")) {
    message = "One of arguments not found";
    code = HTTP_CODE_BAD_REQUEST;
  } else {
    int val = webServer.arg("val").toInt();
    String col = webServer.arg("color");
    int color;
    if (col.equals("RED")) {
      color = RED;
    } else if (col.equals("GREEN")) {
      color = GREEN;
    } else if (col.equals("BLUE")) {
      color = BLUE;
    } else {
      color = RED;
    }

    // Set GPIO according to the request
    digitalWrite(color, val);
    // Prepare the response
    message = "<html>\r\nGPIO " + col + " is now ";
    message += (val) ? "high" : "low";
    message += "</html>\n";
  }

  webServer.sendHeader("Connection", "close");
  webServer.send(code, "text/html", message);  //Returns the HTTP response
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
  size_t htmlSize = strlen(htmlTemplate) + currentTime.length() + 1;
  char html[htmlSize];

  // Format the HTML with the current time
  snprintf(html, htmlSize, htmlTemplate, currentTime.c_str());

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
        <p>Uptime: %02d days %02d:%02d:%02d</p>
        <p><a href='/time'>Time</a></p>
        <p><a href='/restart'>Restart</a></p>
        <p><a href='/ota-update'>OTA</a></p>
        <p><a href='/wifireset'>WiFi Reset</a></p>
        <p><a href='/led'>LED</a></p>
    </div>
</body>
</html>
)rawliteral";

  // Get the size of the htmlPage
  size_t htmlPageSize = strlen(htmlPage);

void handleRoot() {
  char temp[htmlPageSize + 100];
  // Calculate and display the working time
  unsigned long workingTimeMillis = millis();
  unsigned long seconds = (workingTimeMillis / 1000) % 60;
  unsigned long minutes = (workingTimeMillis / (1000 * 60)) % 60;
  unsigned long hours = (workingTimeMillis / (1000 * 60 * 60)) % 24;
  unsigned long days = (workingTimeMillis / (1000 * 60 * 60 * 24));


  snprintf(temp, htmlPageSize + 100, htmlPage, hostname_m.c_str(), version_prg.c_str(), days, hours, minutes, seconds);
  webServer.sendHeader("Connection", "close");
  webServer.send(HTTP_CODE_OK, "text/html", temp);
}

void getTemperature() {
  detachInterrupt_clock_process();

  String tape = "{\"temperature\":" + String(round(homeTemp), 0) + ",\"humidity\":" + String(round(homeHumidity), 0) + "}";
  webServer.sendHeader("Connection", "close");
  webServer.send(200, "application/json", tape);
  init_clock_process();
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
  Serial.printf("Starting HTTP...\n");
  webServer.on("/index.html", HTTP_GET, []() {
    webServer.sendHeader("Connection", "close");
    webServer.send(200, "text/plain", "Hello World!");
  });

  //webServer.begin();

  Serial.printf("Starting SSDP...\n");
  SSDP.setSchemaURL("description.xml");
  SSDP.setHTTPPort(80);
  SSDP.setName("ESP watch");
  SSDP.setSerialNumber("000000001");
  SSDP.setURL("/");
  SSDP.setModelName("ESP Watch 2023");
  SSDP.setModelNumber("01");
  SSDP.setModelURL("https://gerovichev.wixsite.com/gerovichev");
  SSDP.setManufacturer("Gerovichev Electronics");
  SSDP.setManufacturerURL("https://gerovichev.wixsite.com/gerovichev");
  SSDP.begin();

  webServer.on("/description.xml", HTTP_GET, []() {
    SSDP.schema(webServer.client());
  });
}


void webserver_init() {
  if (isWebClientNeeded) {

    ssdp_init();

    webServer.on("/time", handleTime);
    webServer.on("/restart", handleRestart);
    webServer.on("/ota-update", HTTP_GET, handleWebOtaUpdate); // Define a route for triggering the OTA update
    webServer.on("/led", handleLed);
    webServer.on("/wifireset", wifiReset);
    webServer.on("/temperature", getTemperature);
    webServer.on("/", handleRoot);
    webServer.onNotFound(handleNotFound);
    webServer.begin();
  }
}

void webClientHandle() {
  if (isWebClientNeeded) {
    webServer.handleClient();
  }
}
