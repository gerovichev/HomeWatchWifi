// File: MQTTClient.cpp

#include "MQTTClient.h"
#include "dht22_manager.h"
#include "clock.h"
#include "logger.h"
#include "constants.h"

WiFiClient espClient;
PubSubClient client(espClient);

namespace {
unsigned long lastMqttReconnectAttemptMs = 0;
int mqttReconnectFailures = 0;
}

void reconnect() {
  if (client.connected()) {
    mqttReconnectFailures = 0;
    return;
  }

  const unsigned long now = millis();
  if (now - lastMqttReconnectAttemptMs < Timing::MQTT_RECONNECT_DELAY_MS) {
    return;
  }
  lastMqttReconnectAttemptMs = now;

  LOG_DEBUG_F("Attempting MQTT connection...");
  if (client.connect(hostname_m.c_str(), mqtt_user, mqtt_password)) {
    mqttReconnectFailures = 0;
    LOG_INFO_F("MQTT connected successfully");
    return;
  }

  mqttReconnectFailures++;
  LOG_WARNING_FMT("MQTT connection failed, rc=%d", client.state());
  if (mqttReconnectFailures >= Retry::MAX_ATTEMPTS_MQTT) {
    LOG_ERROR_F("Max MQTT reconnection attempts reached; will continue retrying with backoff");
    mqttReconnectFailures = 0;
  }
}


void setup_mqtt() {
  LOG_INFO_FMT("MQTT broker: %s:1883", mqtt_server);
  client.setServer(mqtt_server, 1883);
  LOG_INFO_FMT("MQTT topic: %s", mqtt_topic_str.c_str());
}

void publish_temperature() {
  if (!client.connected()) {
    LOG_DEBUG_F("Skipping MQTT publish: broker is disconnected");
    return;
  }

  Dht22_manager& dht22_manager = Clock::getInstance().getDht22();
  float temperature = dht22_manager.getHomeTemp();
  
  if (isnan(temperature)) {
    LOG_ERROR_F("Failed to read temperature from DHT sensor!");
    return;
  }

  LOG_DEBUG_FMT("Publishing temperature: %.2f C", temperature);

  // snprintf bounds-checks and truncates; dtostrf does not and will write past
  // the end of the buffer for unexpectedly large/garbled sensor values.
  char tempString[Buffer::TEMP_STRING_SIZE];
  snprintf(tempString, sizeof(tempString), "%.2f", temperature);

  if (client.publish(mqtt_topic_str.c_str(), tempString)) {
    LOG_VERBOSE_F("Temperature published to MQTT successfully");
  } else {
    LOG_WARNING_F("Failed to publish temperature to MQTT");
  }
}
