#include <Arduino.h>
#include <FP50_test.h>
#include <SoftwareSerial.h>
#include "FP50.h"
#include "GDBStub.h"
#include "debug.h"

SoftwareSerial fp50Serial;
FP50 fp50(fp50Serial);

struct pt ptMain;
struct pt ptBackground;

PT_THREAD(print_info(struct pt *pt)) {
  static AsyncPT pt1;
  static String ver;

  PT_BEGIN(pt);
  LOGN("Requesting FP50 Version...");

  PT_SPAWN(pt, &pt1.pt, fp50.get_version(pt1, ver));
  LOGNF("FP50 Version: %s", ver.c_str());
  PT_END(pt);
}

/// Entry point of the async main function
/// \param pt
/// \return
PT_THREAD(main_routine(struct pt *pt)) {
  static struct pt ptInfo;
  static struct pt ptTest;
  PT_BEGIN(pt);

  // Print information
  PT_SPAWN(pt, &ptInfo, print_info(&ptInfo));

  PT_SPAWN(pt, &ptTest, run_fp50_tests(&ptTest, fp50));

  // End of main routine.
  LOGV("Main routine exits.");
  while (true) {
    PT_YIELD(pt);
  }
  PT_END(pt);  // PT_END can revive the pt!!
}

PT_THREAD(background_routine(struct pt *pt)) {
  static struct pt ptFP50;
  PT_BEGIN(pt);

  while (true) {
    fp50.daemon(&ptFP50);
    PT_YIELD(pt);
  }

  PT_END(pt);
}

void setup() {
  Serial.begin(115200);
//  gdb_init();

  fp50Serial.begin(9600, SWSERIAL_8N1, D5, D6, false, 100, 100);
  fp50Serial.setTimeout(50);
  fp50.begin();

  LOGV("fp50 serial initialized.");
  PT_INIT(&ptMain);
  PT_INIT(&ptBackground);
}

void loop() {
  // write your code here
  main_routine(&ptMain);
  background_routine(&ptBackground);
}