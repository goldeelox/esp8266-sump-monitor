#define DEBUG

#include "pump.h"
#include "config.h"
#include <Arduino.h>
#include <MQTT.h>
#include <cstdint>
#include <cstdio>

#ifdef ESP32
#include <WiFi.h>
#else
#include <ESP8266WiFi.h>
#endif

MQTTClient mqtt_client;
WiFiClient wifi_client;
const char CLIENT_NAME[] = MQTT_CLIENT_NAME;
const char BROKER_ADDRESS[] = MQTT_BROKER_ADDRESS;
const char TOPIC_NAMESPACE[] = "test/sump";

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

void output(Pump pumps[], size_t size) {
  for (uint8_t i = 0; i < size; i++) {
    char duty_cycle_seconds_topic[64];
    snprintf(duty_cycle_seconds_topic, sizeof(duty_cycle_seconds_topic),
             "%s/%s/running_seconds_total", TOPIC_NAMESPACE, pumps[i].name);

    char duty_cycle_count_topic[64];
    snprintf(duty_cycle_count_topic, sizeof(duty_cycle_count_topic),
             "%s/%s/running_seconds_count", TOPIC_NAMESPACE, pumps[i].name);

#ifdef DEBUG
    Serial.printf("name: %s\n pin: %d\n state: %d\n dutyCycleCount: %d\n "
                  "dutyCycleSeconds: %d\n",
                  pumps[i].name, pumps[i].pin, pumps[i].currentState,
                  pumps[i].dutyCycleCount, pumps[i].dutyCycleSeconds);
#endif
    mqtt_client.publish(duty_cycle_count_topic,
                        String(pumps[i].dutyCycleCount));
    mqtt_client.publish(duty_cycle_seconds_topic,
                        String(pumps[i].dutyCycleSeconds));
  }
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
  output(pumps, pumpsSize);
}
