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
      
      // Attempt to reconnect to saved WiFi
      bool attemptReconnect();
      
  private:
      // Attempt direct connection to saved WiFi credentials
      bool attemptDirectConnection(int maxAttempts);
      
      // Check if WiFi credentials are saved
      bool hasSavedCredentials();
};
