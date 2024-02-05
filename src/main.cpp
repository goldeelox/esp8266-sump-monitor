#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <MQTT.h>
#include <Pump.h>
#include "wifi_credentials.h"

MQTTClient client;
WiFiClient wifi;
const char mqttClientName[] = "esp8266-sump-sensor";
const char mqttBroker[] = "mqtt.hhway.net";
const String topic = "basement/sump";

#define PUMP_A 4
Pump pumpA;

/* monitor pin for 1s */
int senseVibration(uint8_t pin);
/* reconnect to network and mqtt broker */
void reconnect();

void setup() {
  pinMode(PUMP_A, INPUT);
  Serial.begin(115200);
  delay(1000);

  // wifi
  WiFi.begin(W_SSID, W_PASSPHRASE);
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("Connected. IP: ");
  Serial.println(WiFi.localIP());

  // mqtt
  client.begin(mqttBroker, wifi);
}

void loop() {
  client.loop();
  delay(10);

  if (!client.connected()) {
    reconnect();
  }

  int s = senseVibration(PUMP_A);
  pumpA.updateState(s);
  client.publish(topic + "/a/running_seconds_total",
                 String(pumpA.usageSeconds));
  client.publish(topic + "/a/running_seconds_count",
                 String(pumpA.usageCount));
}

int senseVibration(uint8_t pin) {
  int vibrationPeriods = 0;
  int interval = 10; // ms
  for (size_t i = 0; i < 100; i++) {
    vibrationPeriods += digitalRead(pin);
    delay(interval);
  }

  if (vibrationPeriods > 10) {
    return 1;
  }
  return 0;
}

void reconnect() {
  Serial.print("checking wifi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.print("\nconnecting...");
  while (!client.connect(mqttClientName)) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("\nconnected!");
}
