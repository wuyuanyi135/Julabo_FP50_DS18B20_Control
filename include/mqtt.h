//
// Created by wuyua on 2020-09-06.
//

#ifndef JULABO_FP50_DS18B20_CONTROL_MQTT_H
#define JULABO_FP50_DS18B20_CONTROL_MQTT_H
#include <PubSubClient.h>
#include "ESP8266WiFi.h"
#include "FP50.h"
#include "WiFiClient.h"
#include "debug.h"
#include "pt/pt.h"
#include "tmeter.h"

#define WIFI_SSID "DCHost"
#define WIFI_PASS "dchost000000"
#define WIFI_TIMEOUT  10000
#define MQTT_SERVER "192.168.43.1"
#define MQTT_PORT 1883
#define MQTT_DS18_TOPIC "crystallizer/temperature"
#define MQTT_FP50_TEMP_TOPIC "waterbath/temperature"
#define MQTT_FP50_POWER_TOPIC "waterbath/power"
#define MQTT_FP50_SETPOINT_TOPIC "waterbath/setpoint"
#define MQTT_FP50_ENABLE_TOPIC "waterbath/enable"
#define MQTT_LOG_TOPIC "log/TController"

class mqtt {
 private:
  WiFiClient wiFiClient;
  PubSubClient mqttClient{wiFiClient};

 public:
  mqtt(FP50& fp50, TMeter& tm);

 public:
  void begin();
  PT_THREAD(daemon(struct pt* pt));
  void logging(String info);

 public:
  uint ds18Interval;
  uint fp50Interval;

 private:
  uint ds18Last;
  uint fp50Last;

 private:
  FP50& fp50;
  TMeter& tm;

 private:
  PT_THREAD(publish_ds18(struct pt* pt));
  PT_THREAD(publish_fp50(struct pt* pt));

};

#endif  // JULABO_FP50_DS18B20_CONTROL_MQTT_H
