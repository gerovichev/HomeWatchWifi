# HomeWatchWifi Logging System

## Description

The project now uses a centralized logging system with support for log levels, timestamps, and convenient macros.

## Features

- ✅ **5 log levels**: ERROR, WARNING, INFO, DEBUG, VERBOSE
- ✅ **Timestamps**: Automatic addition of time in milliseconds
- ✅ **Convenient macros**: Simple syntax for logging
- ✅ **Flash string support**: RAM savings with F()
- ✅ **Singleton pattern**: Single point of access to logger
- ✅ **Configurable level**: Easy to change log detail

## Log Levels

| Level | Description | When to use |
|-------|------------|-------------|
| **ERROR** | Critical errors | Errors requiring attention |
| **WARNING** | Warnings | Potential problems |
| **INFO** | Informational messages | Key events in operation |
| **DEBUG** | Debug information | Detailed information for debugging |
| **VERBOSE** | Maximum detail | Very detailed information |

## Quick Start

### Initialization

In the `setup()` function:

```cpp
void setup() {
    // Initialize logger
    Logger::getInstance().begin(115200);
    
    // Set log level (optional)
    Logger::getInstance().setLogLevel(LOG_LEVEL_DEBUG);
    
    // Your code...
}
```

### Using Macros

```cpp
// Regular strings
LOG_ERROR("Connection failed: " + String(errorCode));
LOG_WARNING("Low memory: " + String(freeHeap) + " bytes");
LOG_INFO("Device started successfully");
LOG_DEBUG("Temperature: " + String(temp) + "°C");
LOG_VERBOSE("Raw sensor value: " + String(rawValue));

// Flash strings (save RAM)
LOG_ERROR_F("Connection failed");
LOG_WARNING_F("Low memory warning");
LOG_INFO_F("Device started successfully");
LOG_DEBUG_F("Reading sensor...");
LOG_VERBOSE_F("Entering loop iteration");
```

## Usage Examples

### Example 1: WiFi Connection

```cpp
void connectWiFi() {
    LOG_INFO_F("Starting WiFi connection...");
    
    WiFi.begin(ssid, password);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 10) {
        LOG_DEBUG("Connection attempt: " + String(attempts + 1));
        delay(500);
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        LOG_INFO("WiFi connected! IP: " + WiFi.localIP().toString());
    } else {
        LOG_ERROR_F("WiFi connection failed");
    }
}
```

### Example 2: Sensor Reading

```cpp
float readTemperature() {
    LOG_DEBUG_F("Reading temperature sensor...");
    
    float temp = dht.readTemperature();
    
    if (isnan(temp)) {
        LOG_ERROR_F("Failed to read from DHT sensor!");
        return 0.0;
    }
    
    LOG_VERBOSE("Raw temperature value: " + String(temp));
    LOG_INFO("Temperature: " + String(temp, 2) + "°C");
    
    return temp;
}
```

### Example 3: HTTP Request

```cpp
bool fetchWeatherData() {
    LOG_INFO_F("Fetching weather data...");
    
    HTTPClient http;
    http.begin(apiUrl);
    
    LOG_DEBUG("API URL: " + apiUrl);
    
    int httpCode = http.GET();
    
    if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        LOG_VERBOSE("API response: " + payload);
        LOG_INFO_F("Weather data received successfully");
        http.end();
        return true;
    } else {
        LOG_ERROR("HTTP request failed with code: " + String(httpCode));
        http.end();
        return false;
    }
}
```

## Log Level Configuration

### Levels in order of detail

```cpp
Logger::getInstance().setLogLevel(LOG_LEVEL_NONE);     // No logs
Logger::getInstance().setLogLevel(LOG_LEVEL_ERROR);    // Errors only
Logger::getInstance().setLogLevel(LOG_LEVEL_WARNING);  // Errors + warnings
Logger::getInstance().setLogLevel(LOG_LEVEL_INFO);     // + informational (default)
Logger::getInstance().setLogLevel(LOG_LEVEL_DEBUG);    // + debug
Logger::getInstance().setLogLevel(LOG_LEVEL_VERBOSE);  // Everything
```

### Level Recommendations

- **Development**: `LOG_LEVEL_DEBUG` or `LOG_LEVEL_VERBOSE`
- **Testing**: `LOG_LEVEL_INFO` or `LOG_LEVEL_DEBUG`
- **Production**: `LOG_LEVEL_INFO` or `LOG_LEVEL_WARNING`

## Output Format

```
[Timestamp]  [Level]    Message
```

Example:
```
     12345 [INFO]    WiFi connected! IP: 192.168.1.100
     12567 [DEBUG]   Temperature: 23.50°C
     12890 [ERROR]   Sensor read failed
```

## Replacing Old Code

### Before (old style):

```cpp
if (Serial) {
    Serial.print("Temperature: ");
    Serial.print(temp);
    Serial.println(" °C");
}
```

### After (new style):

```cpp
LOG_INFO("Temperature: " + String(temp) + "°C");
```

## Advantages

1. **Uniformity**: All logs in one format
2. **Filtering**: Easy to change detail level
3. **Timestamps**: Track event timing
4. **Readability**: Clear format with levels
5. **Performance**: Logs above set level are not processed
6. **Memory savings**: Flash string support

## Best Practices

1. **Use the correct level**:
   - ERROR for critical errors
   - WARNING for potential problems
   - INFO for important events
   - DEBUG for debug information
   - VERBOSE for maximum detail

2. **Add context**:
   ```cpp
   // Bad
   LOG_ERROR_F("Failed");
   
   // Good
   LOG_ERROR("Sensor read failed: " + String(errorCode));
   ```

3. **Use Flash strings for static strings**:
   ```cpp
   // Good - saves RAM
   LOG_INFO_F("Device initialized");
   
   // Only for dynamic strings
   LOG_INFO("Temp: " + String(temp) + "°C");
   ```

4. **Don't abuse logs**:
   ```cpp
   // Bad - too many logs in loop
   void loop() {
       LOG_VERBOSE_F("Loop iteration");  // Executes thousands of times!
   }
   ```

## Project Migration

Most files have already been updated to use the new logger:
- ✅ main_process.cpp
- ✅ global_config.cpp
- ✅ weather_manager.cpp
- ✅ TimeManager.cpp
- ✅ location_manager.cpp
- ✅ WiFiSetup.cpp
- ✅ OTAUpdate.cpp
- ✅ MQTTClient.cpp
- ✅ clock.cpp

To migrate other files:
1. Add `#include "logger.h"`
2. Replace `if (Serial) Serial.println()` with appropriate macros
3. Choose the correct level for each message

## Disabling Logs

For production build with minimal logs:

```cpp
Logger::getInstance().setLogLevel(LOG_LEVEL_ERROR);
```

Or completely:

```cpp
Logger::getInstance().setLogLevel(LOG_LEVEL_NONE);
```

## Performance

- Logs above the set level are not processed
- Flash strings save RAM
- Minimal overhead when levels are disabled
- Does not affect time-critical code sections

---

**Questions and suggestions**: If you have ideas for improving the logging system, feel free to contribute!
