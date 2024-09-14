#pragma once

#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include "clock.h"

// Global variable declaration
extern String pathOta;

// Function declarations
void update_started();
void update_finished();
void update_progress(int cur, int total);
void update_error(int err);

void web_ota_init();
void update_ota();
