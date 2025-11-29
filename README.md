# HomeWatchWifi

A smart watch project based on ESP8266 with Max72xxPanel LED display that shows time, weather, currency rates, and environmental data.

![License](https://img.shields.io/badge/license-GPLv3-blue.svg)
![Platform](https://img.shields.io/badge/platform-ESP8266-orange.svg)

## Features

- **Time & Date Display**: Real-time clock with NTP synchronization
- **Weather Information**: Current weather, temperature, humidity, pressure from OpenWeatherMap API
- **Currency Rates**: USD and EUR exchange rates
- **Environmental Sensors**: DHT22 temperature and humidity sensor support
- **Location Services**: Automatic location detection via Google Geolocation API
- **Multi-language Support**: Russian and English interface
- **Energy Efficient**: WiFi power management and automatic brightness control
- **OTA Updates**: Over-the-air firmware updates
- **MQTT Integration**: Temperature data publishing to MQTT broker
- **Multi-device Support**: Configuration for multiple devices via MAC addresses
- **Centralized Logging**: Unified logging system with configurable levels
- **Error Handling**: Robust error handling with graceful degradation

## Hardware Requirements

- ESP8266 microcontroller
- Max72xxPanel LED matrix display (4 modules)
- DHT22 temperature/humidity sensor (optional)
- WiFi connection

## Pin Configuration

- **CLK_PIN**: 14 (D5)
- **DATA_PIN**: 13 (D7) 
- **CS_PIN**: 5 (D1)
- **DHTPIN**: 12 (DHT22 sensor)

## Prerequisites

- Arduino IDE or PlatformIO
- ESP8266 board support package
- Required libraries (see below)

## Required Libraries

- `ESP8266WiFi` (included with ESP8266 board package)
- `ESP8266HTTPClient` (included with ESP8266 board package)
- `WiFiClientSecure` (included with ESP8266 board package)
- `ArduinoJson` (for JSON parsing)
- `MD_MAX72XX` (for LED matrix display)
- `MD_Parola` (for text scrolling on display)
- `TimeLib` (for time management)
- `DHT` (for DHT22 sensor)
- `Ticker` (for periodic tasks)

## Setup Instructions

1. **Clone the repository**
   ```bash
   git clone https://github.com/yourusername/HomeWatchWifi.git
   cd HomeWatchWifi
   ```

2. **Configure API Keys**: Create `Secret.cpp` based on `Secret.h` template with your API credentials. Here's an example template:

   ```cpp
   #include "Secret.h"
   
   // API Keys and URLs - Replace with your actual credentials
   const char* googleApiKey = "YOUR_GOOGLE_API_KEY_HERE";
   const char* confPathCurrencyUSD = "https://your-api.com/currency/usd";
   const char* confPathCurrencyEUR = "https://your-api.com/currency/eur";
   const char* confBearerTokenCurrency = "YOUR_BEARER_TOKEN_HERE";
   const char* appidWeather = "YOUR_OPENWEATHER_API_KEY_HERE";
   const char* apiKeyTimezone = "YOUR_TIMEZONE_API_KEY_HERE";
   const char* webOTA_updateURL = "https://your-server.com/ota/update";
   
   // MQTT broker credentials (optional, if using MQTT)
   const char* mqtt_server = "192.168.1.100";
   const char* mqtt_user = "mqtt_username";
   const char* mqtt_password = "mqtt_password";
   const char* mqtt_topic = "/temperature";
   String mqtt_topic_str;
   
   // WiFi credentials for AutoConnectAP (fallback)
   const char* wifi_name = "AutoConnectAP";
   const char* wifi_pass = "your_wifi_password";
   
   // Global device configuration map
   std::map<String, DeviceConfig> configMap;
   
   // Function to set device configurations based on MAC address
   void setDeviceConfig() {
       // Example device configuration
       // Replace MAC address with your device's MAC address
       DeviceConfig config1;
       config1.lang_weather = "en";              // "en" or "ru"
       config1.hostname_m = "ESP_Device1";
       config1.IS_DHT_CONNECTED = true;         // true if DHT22 sensor is connected
       config1.isWebClientNeeded = false;       // true if web client is needed
       config1.isReadWeather = true;            // true to fetch weather data
       config1.humidity_delta = 0.0;             // Humidity correction value
       config1.intensity = 1;                    // Display intensity (0-15)
       config1.isOTAreq = true;                  // true to enable OTA updates
       config1.nameofWatch = "Device Name";     // Name displayed on device
       config1.isMQTT = false;                   // true to enable MQTT
       
       // Add configuration for your device MAC address
       // Find your MAC address in Serial monitor during first boot
       configMap["AA:BB:CC:DD:EE:FF"] = config1;
       
       // Add more device configurations as needed
       // DeviceConfig config2;
       // ... configure settings ...
       // configMap["11:22:33:44:55:66"] = config2;
   }
   ```

   **Important**: 
   - `Secret.cpp` is not tracked by git (it's in `.gitignore`)
   - `Secret.h` is the template file and is safe to publish
   - Never commit your real API keys to the repository
   - To find your device's MAC address, check the Serial monitor during first boot

3. **Device Configuration**: 
   - Modify device configurations in `Secret.cpp` for your specific devices
   - Add your device's MAC address and settings to the `setDeviceConfig()` function
   - You can find your device's MAC address in the Serial monitor during first boot
   - See the example above for all available configuration options

4. **Upload to ESP8266**: 
   - Select your ESP8266 board in Arduino IDE
   - Select the correct COM port
   - Compile and upload the code

5. **WiFi Setup**: 
   - Connect to the AutoConnectAP network
   - Configure your WiFi credentials through the web interface

## API Services Used

- **OpenWeatherMap**: Weather data
- **Google Geolocation API**: Location services
- **TimezoneDB**: Timezone information
- **Custom Currency API**: Exchange rates
- **NTP Servers**: Time synchronization

## Display Cycle

The device cycles through the following information every 5 seconds:
1. Current time
2. Date
3. Day of week
4. Current temperature
5. Feels-like temperature
6. Atmospheric pressure
7. Humidity
8. Weather description
9. USD exchange rate
10. EUR exchange rate
11. Home temperature (DHT22)
12. Home humidity (DHT22)

## Configuration

The project supports multiple device configurations based on MAC addresses. Each device can have different settings for:
- Language (Russian/English)
- Device name
- Sensor connections
- MQTT settings
- OTA update preferences
- Display intensity

## Power Management

- WiFi is automatically disabled after data updates to save power
- Display brightness adjusts based on sunrise/sunset times
- Efficient timer-based operation

## Project Structure

```
HomeWatchWifi/
├── HomeWatchWifi.ino      # Main entry point
├── main_process.cpp/h     # Main process logic
├── clock.cpp/h            # Clock and display management
├── weather_manager.cpp/h  # Weather data handling
├── currency_manager.cpp/h # Currency rates handling
├── location_manager.cpp/h # Location services
├── TimeManager.cpp/h      # Time synchronization
├── dht22_manager.cpp/h    # DHT22 sensor management
├── led_display.cpp/h      # LED display functions
├── logger.cpp/h           # Logging system
├── error_handler.cpp/h    # Error handling
├── device_state.cpp/h     # Device state management
├── constants.h            # Centralized constants
├── secure_client.h        # SSL client configuration
├── global_config.cpp/h    # Global configuration
├── WiFiSetup.cpp/h        # WiFi setup
├── OTAUpdate.cpp/h        # OTA updates
├── MQTTClient.cpp/h       # MQTT client
└── fonts.h                # Font definitions
```

## Documentation

- [CHANGELOG.md](CHANGELOG.md) - Project changelog and improvements
- [LOGGING.md](LOGGING.md) - Logging system documentation

## Troubleshooting

### WiFi Connection Issues
- Check WiFi credentials
- Ensure router is 2.4GHz (ESP8266 doesn't support 5GHz)
- Check signal strength

### Display Issues
- Verify pin connections
- Check display module configuration
- Ensure correct CS pin selection

### Sensor Issues
- Verify DHT22 wiring
- Check sensor power supply
- Ensure sensor is not damaged

### API Issues
- Verify API keys are correct
- Check API quotas/limits
- Ensure internet connection is stable

## Development

### Code Style
- Follow existing code style
- Use meaningful variable names
- Add comments for complex logic
- Keep functions focused and small

### Testing
- Test on hardware before committing
- Verify all features work after changes
- Check memory usage

## License

This project is licensed under the GNU General Public License v3.0 - see the [LICENSE](LICENSE) file for details.

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request. For major changes, please open an issue first to discuss what you would like to change.

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

## Acknowledgments

- ESP8266 community for excellent documentation
- OpenWeatherMap for weather API
- All contributors and testers

## Support

If you encounter any issues or have questions, please open an issue on GitHub.
