#include <Arduino.h>
#include "FP50.h"
#include "pt/pt.h"

HardwareSerial fp50Serial(2);
FP50 fp50(fp50Serial);
struct pt* ptFP50;
struct pt* ptInfo;

PT_THREAD(print_info(struct pt* pt)) {
  static struct pt* pt1;
  static String ver;

  PT_BEGIN(pt);
  PT_SPAWN(pt, pt1, fp50.get_version(pt1, ver));
  Log.verbose("FP50 Version: %s" CR, ver.c_str());
  PT_END(pt);
}

void setup() {
  Log.begin(LOG_LEVEL_VERBOSE, &Serial);
  fp50Serial.begin(9600);
  fp50Serial.setTimeout(50);

  PT_INIT(ptFP50);
  PT_INIT(ptInfo);
}

void loop() {
  // write your code here
  print_info(ptInfo);
  fp50.daemon(ptFP50);
}