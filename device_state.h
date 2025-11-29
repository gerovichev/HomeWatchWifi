#pragma once

#include <Arduino.h>
#include "global_config.h"

/**
 * Класс для управления состоянием устройства
 * Инкапсулирует глобальные переменные состояния
 */
class DeviceState {
public:
    static DeviceState& getInstance();

    // Language
    String getLanguage() const { return lang_weather; }
    void setLanguage(const String& lang) { lang_weather = lang; }

    // Hostname
    String getHostname() const { return hostname_m; }
    void setHostname(const String& hostname) { hostname_m = hostname; }

    // DHT22 connection
    bool isDhtConnected() const { return IS_DHT_CONNECTED; }
    void setDhtConnected(bool connected) { IS_DHT_CONNECTED = connected; }

    // Web client
    bool isWebClientNeeded() const { return ::isWebClientNeeded; }
    void setWebClientNeeded(bool needed) { ::isWebClientNeeded = needed; }

    // Weather reading
    bool isReadWeather() const { return ::isReadWeather; }
    void setReadWeather(bool read) { ::isReadWeather = read; }

    // Humidity delta
    float getHumidityDelta() const { return humidity_delta; }
    void setHumidityDelta(float delta) { humidity_delta = delta; }

    // Watch name
    String getWatchName() const { return nameofWatch; }
    void setWatchName(const String& name) { nameofWatch = name; }

    // OTA
    bool isOtaRequired() const { return isOTAreq; }
    void setOtaRequired(bool required) { isOTAreq = required; }

    // MQTT
    bool isMqttEnabled() const { return isMQTT; }
    void setMqttEnabled(bool enabled) { isMQTT = enabled; }

    // MAC address
    String getMacAddress() const { return macAddrSt; }
    void setMacAddress(const String& mac) { macAddrSt = mac; }

    // Days of week
    String* getDaysOfWeek() { return daysOfTheWeek; }

private:
    DeviceState() = default;
    DeviceState(const DeviceState&) = delete;
    DeviceState& operator=(const DeviceState&) = delete;
};

