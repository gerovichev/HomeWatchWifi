#pragma once

#include <ESP8266SSDP.h>

ESP8266WebServer webServer(80);

void wifiReset() {

  Serial.println("Web reading");

  String message = "";
  int code = HTTP_CODE_OK;

  wifi_reset();
  // Prepare the response
  message = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>\r\nWifi reset \r\n";
  message += "</html>\n";

  webServer.sendHeader("Connection", "close");
  webServer.send(code, "text/html", message);  //Returns the HTTP response

  ESP.restart();
}


void handleRestart() {
  String message = "";

  // Prepare the response
  message = "<html>\r\nRestarting ....";
  message += "</html>\n";

  webServer.sendHeader("Connection", "close");
  webServer.send(HTTP_CODE_OK, "text/html", message);  //Returns the HTTP response

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

void handleTime() {
  String temp;

  temp = String("<html>\
  <head>\
    <meta http-equiv='refresh' content='5'/>\
    <title>ESP8266 Demo</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
    </style>\
  </head>\
  <body>\
    <h1>Hello from ESP8266!</h1>\
    <p>Current time: ")
         + timeClient.getFormattedTime() + String("</p>\
  </body>\
</html>");

  webServer.sendHeader("Connection", "close");
  webServer.send(200, "text/html", temp);
}

void handleRoot() {
  char temp[800];
  int sec = millis() / 1000;
  int min = sec / 60;
  int hr = min / 60;

  snprintf(temp, 800,

           "<?xml version='1.0' encoding='UTF-8'?>\
              <!DOCTYPE html>\
              <html xmlns='http://www.w3.org/1999/xhtml'>\
             <head>\
    <meta http-equiv='refresh' content='5'/>\
    <title>ESP8266 Demo</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
    </style>\
  </head>\
  <body>\
    <div width='100%'>\
    <h1>Hello from ESP8266! %s - %s</h1>\
    <p>Uptime: %02d:%02d:%02d</p>\
    <p><a href='/time'>time</a></p>\
    <p><a href='/restart'>restart</a></p>\
    <p><a href='/wifireset'>wifireset</a></p>\
    <p><a href='/led'>led</a></p>\
    </div>\
  </body>\
</html>",

           hostname_m.c_str(), version_prg.c_str(), hr, min % 60, sec % 60);
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
