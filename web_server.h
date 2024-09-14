#pragma once

#include <ESP8266WebServer.h>
#include <ESP8266SSDP.h>
#include "OTAUpdate.h"

extern ESP8266WebServer webServer;

void sendDynamicHTMLResponse(const char* title, const char* message, int refreshTime);
void wifiReset();
void handleRestart();
void handleTime();
void handleRoot();
void handleNotFound();
void handleWebOtaUpdate();
void ssdp_init();
void webserver_init();
void webClientHandle();
void updateTotalMillis();

extern unsigned long prevMillis;
extern unsigned long totalMillis;
