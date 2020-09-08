//
// Created by wuyua on 2020-09-06.
//

#include "mqtt.h"

PT_THREAD(mqtt::begin(struct pt* pt)) {
  static ulong now = 0;
  static int cnt = 0;
  PT_BEGIN(pt);
  if (WiFi.getAutoConnect()) {
    WiFi.setAutoConnect(false);
  }
  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  now = millis();
  while (WiFi.status() != WL_CONNECTED && cnt < WIFI_TIMEOUT_S) {
    LOGN("WiFi is connecting..");
    cnt++;
    PT_YIELD_UNTIL(pt, millis() - now > 1000);
    now = millis();
  }

  if (cnt == WIFI_TIMEOUT_S) {
    LOGE("Failed to connect to WiFi");
    PT_EXIT(pt);
  }

  // Connect to MQTT
  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  mqttClient.setCallback([&](char* topic, byte* payload, unsigned int length) {
    String p = String((char*)payload);
    LOGVF("Topic: %s; Length: %u;", topic, length);
    if (0 == strcmp(topic, MQTT_FP50_SETPOINT_TOPIC)) {
      double d = String((char*)payload).toDouble();
      fp50.set_setpoint(d, 0);
    } else if (0 == strcmp(topic, MQTT_FP50_ENABLE_TOPIC)) {
      int d = String((char*)payload).toInt();
      fp50.switch_power(d);
    }
  });

  mqttClient.connect(String(ESP.getChipId()).c_str());

  now = millis();
  while (!mqttClient.connected()) {
    LOGV("MQTT connecting");
    PT_YIELD_UNTIL(pt, millis() - now > 1000);
  }

  // After connect
  if(!mqttClient.subscribe(MQTT_FP50_SETPOINT_TOPIC)) LOGE("Failed to subscribe setpoint");
  if(!mqttClient.subscribe(MQTT_FP50_ENABLE_TOPIC)) LOGE("Failed to subscribe enable");

  PT_END(pt);
}

// @ singleton
PT_THREAD(mqtt::daemon(struct pt* pt)) {
  static ulong now;
  static struct pt pt1, pt2;
  PT_BEGIN(pt);
  while (true) {
    if (mqttClient.connected()) {
      publish_ds18(&pt1);
      publish_fp50(&pt2);
      mqttClient.loop();
    } else {
      // ERROR Handling
    }
    PT_YIELD(pt);
  }
  PT_END(pt);
}

mqtt::mqtt(FP50& fp50, TMeter& tm) : fp50(fp50), tm(tm) {}

char mqtt::publish_ds18(struct pt* pt) {
  static TMeterPT pt1;
  static float temp;
  PT_BEGIN(pt);
  if (millis() - ds18Last > ds18Interval) {
    PT_SPAWN(pt, &pt1.pt, tm.get_temperature(pt1, temp));
    mqttClient.publish(MQTT_DS18_TOPIC, String(temp).c_str(), false);
    LOGV("Published DS18 message");
    ds18Last = millis();
  }
  PT_END(pt);
}
char mqtt::publish_fp50(struct pt* pt) {
  static uint8 r1 = 0, r2 = 0;
  static AsyncPT pt1, pt2;
  static double temp, power;
  PT_BEGIN(pt);
  if (!fp50.ok) PT_EXIT(pt);

  if (millis() - fp50Last > fp50Interval) {
    r1 = 0;
    r2 = 0;
    do {
      if (PT_SCHEDULE(r1)) r1 = fp50.get_bath_temperature(pt1, temp);
      if (PT_SCHEDULE(r2)) r2 = fp50.get_heating_power(pt2, power);
      PT_YIELD(pt);
    } while (PT_SCHEDULE(r1) && PT_SCHEDULE(r2));

    mqttClient.publish(MQTT_FP50_TEMP_TOPIC, String(temp).c_str(), false);
    mqttClient.publish(MQTT_FP50_POWER_TOPIC, String(power).c_str(), false);
    LOGV("Published fp50 message");
    fp50Last = millis();
  }
  PT_END(pt);
}
void mqtt::logging(String info) {
  if (mqttClient.connected()) {
    mqttClient.publish(MQTT_LOG_TOPIC, info.c_str(), true);
  }
}
