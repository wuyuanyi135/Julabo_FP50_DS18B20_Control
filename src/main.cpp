
#include <Arduino.h>
#include <FP50_test.h>
#include <SoftwareSerial.h>
#include "FP50.h"
#include "GDBStub.h"
#include "PropertyNode.h"
#include "PubSubClientInterface.h"
#include "SimpleCLIInterface.h"
#include "debug.h"
#include "led_state.h"
#include "mqtt.h"
#include "tmeter.h"

#define WIFI_MAX_TRIAL 30
#define MQTT_MAX_TRIAL 30

SoftwareSerial fp50Serial;
FP50 fp50(fp50Serial);
TMeter tm;
WiFiClient wiFiClient;
String buf;
SimpleCLI cli;
SimpleCLIInterface cliInterface(cli, Serial);
PubSubClient client{wiFiClient};
PubSubClientInterface mqttInterface(client, "", true, true);
PropertyNode<double> setpoint("setpoint", 0);  // Do not send initial value
PropertyNode<std::string> event("event", "", true);
PropertyNode<float> crystallizer_temperature("crystallizer_temperature", 0,
                                             true);
PropertyNode<double> internal_temperature("internal_temperature", 0, true);
PropertyNode<double> power("power", 0, true);
PropertyNode<bool> start("start", false);

bool shouldGetTM = false;
bool shouldGetPower = false;
bool shouldGetInternalTemperature = false;

struct pt ptFP50;
struct pt ptBroker;
TMeterPT ptTM;
AsyncPT ptFP50Task;
struct pt ptBrokerStart;
char retBrokerStart = 0;

void setup() {
  Serial.begin(115200);
  led.begin();

  if (WiFi.getAutoConnect()) {
    WiFi.setAutoConnect(false);
  }
  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  int cnt = 0;
  while (WiFi.status() != WL_CONNECTED && cnt < WIFI_MAX_TRIAL) {
    Serial.println("WiFi is connecting..");
    delay(1000);
    cnt++;
  }
  if (cnt == WIFI_MAX_TRIAL) {
    Serial.println("Failed to connect to WiFi");
    ESP.restart();
  }
  client.setServer(MQTT_SERVER, MQTT_PORT);
  const String& id = String(ESP.getChipId());
  client.connect(id.c_str(), (id + "/status").c_str(), "off");
  cnt = 0;
  while (!client.connected() && cnt < MQTT_MAX_TRIAL) {
    Serial.println("MQTT is connecting..");
    delay(1000);
    cnt++;
  }
  if (cnt == MQTT_MAX_TRIAL) {
    Serial.println("Failed to connect to MQTT");
    ESP.restart();
  }
  Serial.println("Connection initialization completed");

  client.publish((id + "/desc").c_str(), "FP50 Controller", true);
  client.publish((id + "/status").c_str(), "on", true);
  Serial.println("Published initial messages");

  setpoint.register_interface(mqttInterface);
  setpoint.register_interface(cliInterface);
  setpoint.set_validator(
      [](double value) { return (value > -20) && (value < 150); });
  setpoint.set_update_callback([](double oldVal, double newVal) {
    if (oldVal == newVal) {
      return;
    }
    fp50.set_setpoint(newVal, 0);
  });
  Serial.println("Setpoint registered");

  event.register_interface(mqttInterface);
  event.register_interface(cliInterface);
  Serial.println("Event registered");

  crystallizer_temperature.register_interface(mqttInterface);
  crystallizer_temperature.set_get_callback([]() {
    shouldGetTM = true;
    return false;
  });
  Serial.println("crystallizer_temperature registered");

  power.register_interface(mqttInterface);
  power.register_interface(cliInterface);
  power.set_get_callback([]() {
    shouldGetPower = true;
    return false;
  });
  Serial.println("power registered");
  internal_temperature.register_interface(mqttInterface);
  internal_temperature.register_interface(cliInterface);
  internal_temperature.set_get_callback([]() {
    shouldGetInternalTemperature = true;
    return false;
  });
  Serial.println("internal_temperature registered");

  start.register_interface(mqttInterface);
  start.register_interface(cliInterface);
  start.set_update_callback([](bool oldVal, bool newVal) {
    if (oldVal == newVal) {
      return;
    }
    fp50.switch_power(newVal);
  });
  Serial.println("start registered");

  fp50Serial.begin(9600, SWSERIAL_8N1, D5, D6, false, 100, 100);
  fp50Serial.setTimeout(50);
  Serial.println("FP50 Serial initialized");

  fp50.begin();
  Serial.println("FP50 initialized");

  tm.begin();
  Serial.println("DS18 initialized");

  PT_INIT(&ptFP50);
  Serial.println("Operation starts");
}

void loop() {
  client.loop();
  while (Serial.available()) {
    // Read out string from the serial monitor
    char ch = (char)Serial.read();
    buf += ch;
    // Parse the user input into the CLI
    if (ch == '\n') {
      cli.parse(buf);
      buf.clear();
    }
  }
  led.update();
  fp50.daemon(&ptFP50);
  if (shouldGetTM) {
    if (tm.get_temperature(ptTM, *crystallizer_temperature.data()) ==
        PT_ENDED) {
      // Done
      crystallizer_temperature.notify_get_request_completed();
      shouldGetTM = false;
    }
  }
  if (shouldGetPower) {
    if (fp50.get_heating_power(ptFP50Task, *power.data()) == PT_ENDED) {
      power.notify_get_request_completed();
      shouldGetPower = false;
    }
  } else if (shouldGetInternalTemperature) {
    // Else if ensure only one is running so share the PT
    if (fp50.get_bath_temperature(ptFP50Task, *internal_temperature.data()) ==
        PT_ENDED) {
      internal_temperature.notify_get_request_completed();
      shouldGetInternalTemperature = false;
    }
  }
}