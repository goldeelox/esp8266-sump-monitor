#define DEBUG

#include "config.h"
#include "pump.h"
#include <Arduino.h>
#include <MQTT.h>

#ifdef ESP32
#include <WiFi.h>
#else
#include <ESP8266WiFi.h>
#endif

MQTTClient mqtt_client;
WiFiClient wifi_client;
const char CLIENT_NAME[] = MQTT_CLIENT_NAME;
const char BROKER_ADDRESS[] = MQTT_BROKER_ADDRESS;
const char TOPIC_NAMESPACE[] = MQTT_TOPIC_NAMESPACE;

// pumps
Pump pumps[] = {{"primary", 26}, {"secondary", 27}};
size_t pumpsSize = sizeof(pumps) / sizeof(Pump);

void senseVibration(Pump pumps[], size_t size) {
  const uint8_t THRESHOULD = 10;
  uint8_t sensors[4] = {};

  // read pin every 10ms for a second
  for (uint8_t i = 0; i < 100; i++) {
    for (uint8_t s = 0; s < size; s++) {
      sensors[s] += digitalRead(pumps[s].pin);
    }
    delay(10);
  }

  // update pump state
  for (uint8_t s = 0; s < size; s++) {
    uint8_t active = 0;
    if (sensors[s] > THRESHOULD) {
      active = 1;
    };
    updateState(&pumps[s], active);
  }
};

void mqtt_publish(Pump pump) {
  char topic[64];
  char payload[16];

  // total seconds
  snprintf(topic, sizeof(topic), "%s/%s/running_seconds_total", TOPIC_NAMESPACE,
           pump.name);
  snprintf(payload, sizeof(payload), "%d", pump.dutyCycleSeconds);
  mqtt_client.publish(topic, payload);

  // count
  snprintf(topic, sizeof(topic), "%s/%s/running_seconds_count", TOPIC_NAMESPACE,
           pump.name);
  snprintf(payload, sizeof(payload), "%d", pump.dutyCycleCount);
  mqtt_client.publish(topic, payload);
}

void reconnectNetwork() {
  Serial.print("checking wifi connection..");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.printf("connected! (IP: %s)\n", WiFi.localIP().toString());
}

void reconnectMqtt() {
  Serial.print("connecting to mqtt broker..");
  while (!mqtt_client.connect(CLIENT_NAME)) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("connected!\n");
}

void setup() {
  Serial.begin(115200);
  delay(2500);

  for (uint8_t i = 0; i < pumpsSize; i++) {
    pinMode(pumps[i].pin, INPUT);
  }

  // wifi
  WiFi.begin(WIFI_SSID, WIFI_PASSPHRASE);

  // mqtt
  mqtt_client.begin(BROKER_ADDRESS, wifi_client);
}

void loop() {
  mqtt_client.loop();
  delay(10);

  if (!mqtt_client.connected()) {
    reconnectNetwork();
    reconnectMqtt();
  }

  senseVibration(pumps, pumpsSize);

  for (size_t i = 0; i < pumpsSize; i++) {
    mqtt_publish(pumps[i]);
#ifdef DEBUG
    Serial.printf("name: %s\n pin: %d\n state: %d\n dutyCycleCount: %d\n "
                  "dutyCycleSeconds: %d\n",
                  pumps[i].name, pumps[i].pin, pumps[i].currentState,
                  pumps[i].dutyCycleCount, pumps[i].dutyCycleSeconds);
#endif
  }
}
