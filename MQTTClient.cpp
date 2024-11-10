// File: MQTTClient.cpp

#include "MQTTClient.h"
#include "dht22_manager.h"
#include "clock.h"

WiFiClient espClient;
PubSubClient client(espClient);

void reconnect() {
  int attemptCount = 0;
  const int maxAttempts = 3;

  while (!client.connected() && attemptCount < maxAttempts) {
    if (Serial) Serial.print(F("Attempting MQTT connection..."));
    if (client.connect(hostname_m.c_str(), mqtt_user, mqtt_password)) {
      if (Serial) Serial.println(F("connected"));
    } else {
      if (Serial) 
      {
        Serial.print(F("failed, rc="));
        Serial.print(client.state());
        Serial.println(F(" try again in 5 seconds"));
      }
      delay(5000);
      attemptCount++;
    }
  }

  if (attemptCount >= maxAttempts && Serial ) {
    Serial.println(F("Max connection attempts reached. Could not connect to MQTT broker."));
  }
}


void setup_mqtt() {
  client.setServer(mqtt_server, 1883);
  if (Serial) Serial.println(F("Topic name ") + mqtt_topic_str);
}

void publish_temperature() {
  Dht22_manager& dht22_manager = Clock::getInstance().getDht22();
  float temperature = dht22_manager.getHomeTemp();
  
  if (isnan(temperature)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  if (Serial) 
  {
    Serial.print(F("Temperature: "));
    Serial.print(temperature);
    Serial.println(F(" *C"));
  }

  char tempString[8];
  dtostrf(temperature, 1, 2, tempString);
  client.publish(mqtt_topic_str.c_str(), tempString);
}
