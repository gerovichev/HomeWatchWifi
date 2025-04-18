// File: MQTTClient.h

#pragma once

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

extern String hostname_m;
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
void publish_temperature();
