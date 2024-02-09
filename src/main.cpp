/* #define DEBUG */
#define PRIMARY_PUMP_PIN 4
#define SECONDARY_PUMP_PIN 5

#include "wifi_credentials.h"
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <MQTT.h>
#include <Pump.h>
#include <cstdint>

MQTTClient client;
WiFiClient wifi;
const char mqttClientName[] = "esp8266-sump-sensor";
const char mqttBroker[] = "mqtt.hhway.net";
const String topic = "basement/sump";

// pumps
const char primaryPumpName[] = "primary";
Pump primaryPump(primaryPumpName, PRIMARY_PUMP_PIN);

#ifdef SECONDARY_PUMP_PIN
const char secondaryPumpName[] = "secondary";
Pump secondaryPump(secondaryPumpName, SECONDARY_PUMP_PIN);
#endif

int senseVibration(uint8_t pin);
void senseVibration(Pump &pumpA, Pump &pumpB);
void reconnectMqtt();
void reconnectNetwork();
void outputState(Pump p);

void setup() {
  Serial.begin(115200);
  delay(1000);

  // wifi
  WiFi.begin(W_SSID, W_PASSPHRASE);
  reconnectNetwork();
  Serial.println(WiFi.localIP());

  // mqtt
  client.begin(mqttBroker, wifi);
}

void loop() {
  client.loop();
  delay(10);

  if (!client.connected()) {
    reconnectNetwork();
    reconnectMqtt();
  }

#ifndef SECONDARY_PUMP_PIN
  int result = senseVibration(PRIMARY_PUMP_PIN);
  primaryPump.updateState(result);
#else
  senseVibration(primaryPump, secondaryPump);
  outputState(secondaryPump);
#endif
  outputState(primaryPump);
}

void outputState(Pump p) {
#ifdef DEBUG
  Serial.print(topic + "/" + p.name + "/running_seconds_total = ");
  Serial.println(p.usageSeconds);
  Serial.print(topic + "/" + p.name + "/running_seconds_count = ");
  Serial.println(p.usageCount);
#else
  client.publish(topic + "/" + p.name + "/running_seconds_total",
                 String(p.usageSeconds));
  client.publish(topic + "/" + p.name + "/running_seconds_count",
                 String(p.usageCount));
#endif
}

int senseVibration(uint8_t pin) {
  int vibrationPeriods = 0;

  // read pin every 10ms over the next 1s
  for (size_t i = 0; i < 100; i++) {
    vibrationPeriods += digitalRead(pin);
    delay(10);
  }

  if (vibrationPeriods > 10) {
    return 1;
  }
  return 0;
}

void senseVibration(Pump &pumpA, Pump &pumpB) {
  int periods[2] = {0, 0};
  int aState = 0;
  int bState = 0;

  // read pins every 10ms for the next 1s
  for (size_t i = 0; i < 100; i++) {
    periods[0] += digitalRead(pumpA.pin);
    periods[1] += digitalRead(pumpB.pin);
    delay(10);
  }

  if (periods[0] > 10) {
    aState = 1;
  }
  if (periods[1] > 10) {
    bState = 1;
  }
  pumpA.updateState(aState);
  pumpB.updateState(bState);
}

void reconnectNetwork() {
  Serial.print("\nchecking wifi connection..");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("connected!");
}

void reconnectMqtt() {
  Serial.print("\nconnecting to mqtt broker..");
  while (!client.connect(mqttClientName)) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("connected!");
}
