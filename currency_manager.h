#pragma once

#include "global_config.h"

// Global variables for currency data
extern float data_USD_value;
extern float data_EUR_value;

// Function prototypes
float readCurrency(String path);
void printCurrencyUSDToScreen();
void printCurrencyEURToScreen();
void currency_init();
