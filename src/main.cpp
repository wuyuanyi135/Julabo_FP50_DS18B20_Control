#include <Arduino.h>
#include <FP50_test.h>
#include <SoftwareSerial.h>
#include "FP50.h"
#include "GDBStub.h"
#include "debug.h"
#include "led_state.h"
#include "mqtt.h"
#include "tmeter.h"

SoftwareSerial fp50Serial;
FP50 fp50(fp50Serial);
TMeter tm;
mqtt broker(fp50, tm);

struct pt ptFP50;
struct pt ptBroker;
struct pt ptBrokerStart;
char retBrokerStart = 0;

void debug_write_helper(bool remote, const char* fmt, ...) {
  const int bufsize = 128;
  char buf[bufsize];
  va_list args;
  va_start(args, fmt);
  vsnprintf(buf, bufsize, fmt, args);
  va_end(args);

  Serial.print(buf);
  Serial.flush();

  if (remote) broker.logging(buf);
}

void setup() {
  Serial.begin(115200);
  led.begin();
  //  gdb_init();

  fp50Serial.begin(9600, SWSERIAL_8N1, D5, D6, false, 100, 100);
  fp50Serial.setTimeout(50);
  fp50.begin();
  LOGV("fp50 serial initialized.");

  tm.begin();
  LOGV("TMeter initialized.");
}

void loop() {
  PT_ONCE(retBrokerStart, broker.begin(&ptBrokerStart));
  led.update();
  fp50.daemon(&ptFP50);
  broker.daemon(&ptBroker);
}