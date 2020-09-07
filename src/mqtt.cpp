//
// Created by wuyua on 2020-09-06.
//

#include "mqtt.h"


void mqtt::begin() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  const int8_t result = WiFi.waitForConnectResult(WIFI_TIMEOUT);
  if (result == -1) {
    LOGE("Failed to connect to WiFi");
    return;
  }

  LOGNF("IP: %s", WiFi.localIP().toString().c_str());

  // Connect to MQTT
  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  mqttClient.setCallback([&](char* topic, byte* payload, unsigned int length) {
    String p = String((char*)payload);
    LOGVF("Topic: %s; Length: %u; Payload=%s", topic, length, p.c_str());
    if (0 == strcmp(topic, MQTT_FP50_SETPOINT_TOPIC)) {
      double d = String((char*)payload).toDouble();
      fp50.set_setpoint(d, 0);
    } else if (0 == strcmp(topic, MQTT_FP50_ENABLE_TOPIC)) {
      int d = String((char*)payload).toInt();
      fp50.switch_power(d);
    }

  });

  mqttClient.connect(String(ESP.getChipId()).c_str());
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
  if (millis() - fp50Last > fp50Interval) {
    do {
      if (PT_SCHEDULE(r1)) r1 = fp50.get_bath_temperature(pt1, temp);
      if (PT_SCHEDULE(r2)) r2 = fp50.get_heating_power(pt2, power);
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
