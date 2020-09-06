//
// Created by wuyuanyi on 2020-09-06.
//

#include "FP50_test.h"
#include "debug.h"
PT_THREAD(run_fp50_tests(struct pt* pt, FP50& fp50)) {
  static AsyncPT pt1;
  static String tmp;
  static ulong now;
  static double dtmp;
  static int i;
  PT_BEGIN(pt);
#ifndef FP50_TEST_DISABLED
  LOGN("=== Running FP50 Test Program === ");
  now = millis();
  pt1.init();
  tmp.clear();
  PT_SPAWN(pt, &pt1.pt, fp50.get_version(pt1, tmp));
  LOGNF("FP50 version: %s (%lu)", tmp.c_str(), millis() - now);
  now = millis();
  PT_YIELD_UNTIL(pt, millis() - now > 5000);
  now = millis();
  pt1.init();
  PT_SPAWN(pt, &pt1.pt, fp50.get_status(pt1, tmp));
  LOGNF("Status: %s (%lu)", tmp.c_str(), millis() - now);
  now = millis();
  PT_YIELD_UNTIL(pt, millis() - now > 5000);

  // On Off
  {
    now = millis();
    fp50.switch_power(false);
    LOGNF("Switched off (%lu)", millis() - now);
    now = millis();
    PT_YIELD_UNTIL(pt, millis() - now > 5000);

    now = millis();
    pt1.init();
    tmp.clear();
    PT_SPAWN(pt, &pt1.pt, fp50.get_status(pt1, tmp));
    LOGNF("Status: %s (%lu)", tmp.c_str(), millis() - now);
    now = millis();
    PT_YIELD_UNTIL(pt, millis() - now > 5000);

    now = millis();
    fp50.switch_power(true);
    LOGNF("Switched on (%lu)", millis() - now);
    now = millis();
    PT_YIELD_UNTIL(pt, millis() - now > 5000);

    now = millis();
    pt1.init();
    tmp.clear();
    PT_SPAWN(pt, &pt1.pt, fp50.get_status(pt1, tmp));
    LOGNF("Status: %s (%lu)", tmp.c_str(), millis() - now);
    now = millis();
    PT_YIELD_UNTIL(pt, millis() - now > 5000);
  }

  // set points
  {
    for (i = 0; i < 3; ++i) {
      now = millis();
      fp50.set_setpoint(27.0 + i, i);
      LOGNF("Setpoint %d set to: %f (%lu)", i, 27.0 + i, millis() - now);
      now = millis();
      PT_YIELD_UNTIL(pt, millis() - now > 5000);

      now = millis();
      pt1.init();
      PT_SPAWN(pt, &pt1.pt, fp50.get_setpoint(pt1, i, dtmp));
      LOGNF("Get setpoint %d: %f (%lu)", i, dtmp, millis() - now);
      now = millis();
      PT_YIELD_UNTIL(pt, millis() - now > 5000);

      now = millis();
      fp50.select_setpoint(i);
      LOGNF("Select setpoint %d (%lu)", i, millis() - now);
      now = millis();
      PT_YIELD_UNTIL(pt, millis() - now > 5000);
    }

    // Pump stage
    {
      for (i = 1; i < 4; i++) {
        now = millis();
        fp50.set_pump_pressure(i);
        LOGNF("Pump stage: %d (%lu)", i, millis() - now);
        now = millis();
        PT_YIELD_UNTIL(pt, millis() - now > 5000);

        now = millis();
        pt1.init();
        PT_SPAWN(pt, &pt1.pt, fp50.get_pump_stage(pt1, dtmp));
        LOGNF("Get pump stage: %f (%lu)", dtmp, millis() - now);
        now = millis();
        PT_YIELD_UNTIL(pt, millis() - now > 5000);
      }
    }
  }
#endif
  PT_END(pt);
}