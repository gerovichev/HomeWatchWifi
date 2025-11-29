#pragma once

#include <ESP8266WiFi.h>

class WIFISetup
{
  public:
      // Function declarations for WiFi setup
      void wifi_init();
      void wifi_reset();
      
      // Check WiFi connection status
      static bool isConnected();
      
      // Get WiFi status string
      static String getStatusString();
};
