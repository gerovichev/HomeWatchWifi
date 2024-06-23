// File: MQTTClient.h

#ifndef MQTTCLIENT_H
#define MQTTCLIENT_H

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// MQTT broker credentials
extern const char* mqtt_server;
extern const char* mqtt_user;
extern const char* mqtt_password;
extern const String mqtt_topic;
extern String mqtt_topic_str;

extern WiFiClient espClient;
extern PubSubClient client;

void reconnect();
void setup_mqtt();
void publish_temperature(float homeTemper);

#endif