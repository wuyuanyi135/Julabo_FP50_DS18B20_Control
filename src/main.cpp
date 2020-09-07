#include <Arduino.h>
#include <FP50_test.h>
#include <SoftwareSerial.h>
#include "FP50.h"
#include "GDBStub.h"
#include "debug.h"
#include "led_state.h"
#include "tmeter.h"
#include "mqtt.h"

#define PT_NO_REVIVE(r, f) \
  if (PT_SCHEDULE(r)) r = f;

SoftwareSerial fp50Serial;
FP50 fp50(fp50Serial);
TMeter tm;
mqtt broker(fp50, tm);

bool fp50OK = false;

struct pt ptFP50;
struct pt ptBroker;

uint8 retFP50Startup = 0;

PT_THREAD(fp50_startup(struct pt *pt)) {
  static struct pt ptInfo;
  static struct pt ptTest;
  static AsyncPT pt1;
  static String ver;
  PT_BEGIN(pt);
  // Print information and determine connection
  LOGN("Requesting FP50 Version...");
  PT_SPAWN(pt, &pt1.pt, fp50.get_version(pt1, ver));

  if (ver.isEmpty()) {
    LOGE("FP50 failed to communicate");
    led.error();
    fp50OK = false;
  } else {
    LOGNF("FP50 Version: %s", ver.c_str());
  }

  if (fp50OK) {
    // when fp50 is ready.
    PT_SPAWN(pt, &ptTest, run_fp50_tests(&ptTest, fp50));
  }

  // End of main routine.
  LOGV("fp50 startup.");

  PT_END(pt);
}

void debug_write_helper(const char* fmt, ...) {
  char buf[128];
  va_list args;
  va_start(args, fmt);

  snprintf(buf, 128, fmt, args);

  Serial.print(buf);

  broker.logging(buf);
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

  broker.begin();
  LOGV("MQTT initialized.");
}

void loop() {
  // Once exit, no longer re-enter.
  PT_NO_REVIVE(retFP50Startup, fp50_startup(&ptFP50));

  led.update();
  fp50.daemon(&ptFP50);
  broker.daemon(&ptBroker);
}