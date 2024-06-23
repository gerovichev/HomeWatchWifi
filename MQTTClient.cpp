// File: MQTTClient.cpp

#include "MQTTClient.h"

WiFiClient espClient;
PubSubClient client(espClient);

void reconnect() {
  int attemptCount = 0;
  const int maxAttempts = 3;

  while (!client.connected() && attemptCount < maxAttempts) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP8266Client", mqtt_user, mqtt_password)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
      attemptCount++;
    }
  }

  if (attemptCount >= maxAttempts) {
    Serial.println("Max connection attempts reached. Could not connect to MQTT broker.");
  }
}


void setup_mqtt() {
  client.setServer(mqtt_server, 1883);
  Serial.println("Topic name " + mqtt_topic_str);
}

void publish_temperature(float homeTemper) {
  float temperature = homeTemper;
  
  if (isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" *C");

  char tempString[8];
  dtostrf(temperature, 1, 2, tempString);
  client.publish(mqtt_topic_str.c_str(), tempString);
}
