//
// Created by wuyuanyi on 2020-09-06.
//

#include "FP50_test.h"
PT_THREAD(run_fp50_tests(struct pt* pt, FP50& fp50)) {
  static AsyncPT pt1;
  static String tmp;
  static ulong now;
  static double dtmp;
  static int i;
  PT_BEGIN(pt);
#ifndef FP50_TEST_DISABLED
  Log.notice("=== Running FP50 Test Program === " CR);
  now = millis();
  pt1.init();
  fp50.get_version(pt1, tmp);
  PT_SEM_WAIT(pt, &pt1.sem);
  Log.notice("FP50 version: %s (%u)" CR, tmp.c_str(), millis() - now);
  now = millis();
  PT_YIELD_UNTIL(pt, millis() - now > 1000);

  now = millis();
  pt1.init();
  fp50.get_status(pt1, tmp);
  PT_SEM_WAIT(pt, &pt1.sem);
  Log.notice("Status: %s (%u)" CR, tmp.c_str(), millis() - now);
  now = millis();
  PT_YIELD_UNTIL(pt, millis() - now > 1000);

  // On Off
  {
    now = millis();
    fp50.switch_power(false);
    Log.notice("Switched off (%u)" CR, millis() - now);
    now = millis();
    PT_YIELD_UNTIL(pt, millis() - now > 3000);

    now = millis();
    pt1.init();
    fp50.get_status(pt1, tmp);
    PT_SEM_WAIT(pt, &pt1.sem);
    Log.notice("Status: %s (%u)" CR, tmp.c_str(), millis() - now);
    now = millis();
    PT_YIELD_UNTIL(pt, millis() - now > 1000);

    now = millis();
    fp50.switch_power(true);
    Log.notice("Switched on (%u)" CR, millis() - now);
    now = millis();
    PT_YIELD_UNTIL(pt, millis() - now > 3000);

    now = millis();
    pt1.init();
    fp50.get_status(pt1, tmp);
    PT_SEM_WAIT(pt, &pt1.sem);
    Log.notice("Status: %s (%u)" CR, tmp.c_str(), millis() - now);
    now = millis();
    PT_YIELD_UNTIL(pt, millis() - now > 1000);
  }

  // set points
  {
    for (i = 0; i < 3; ++i) {
      now = millis();
      fp50.set_setpoint(27.0 + i, i);
      Log.notice("Setpoint %d set to: %D (%u)" CR, i, 27.0 + i, millis() - now);
      now = millis();
      PT_YIELD_UNTIL(pt, millis() - now > 3000);

      now = millis();
      pt1.init();
      fp50.get_setpoint(pt1, i, dtmp);
      PT_SEM_WAIT(pt, &pt1.sem);
      Log.notice("Get setpoint %d: %D (%u)" CR, i, dtmp, millis() - now);
      now = millis();
      PT_YIELD_UNTIL(pt, millis() - now > 1000);

      now = millis();
      fp50.select_setpoint(i);
      Log.notice("Select setpoint %d (%u)" CR, i, millis() - now);
      now = millis();
      PT_YIELD_UNTIL(pt, millis() - now > 3000);
    }

    // Pump stage
    {
      for (i = 1; i < 4; i++) {
        now = millis();
        fp50.set_pump_pressure(i);
        Log.notice("Pump stage: %d (%u)" CR, i, millis() - now);
        now = millis();
        PT_YIELD_UNTIL(pt, millis() - now > 5000);

        now = millis();
        pt1.init();
        fp50.get_pump_stage(pt1, dtmp);
        PT_SEM_WAIT(pt, &pt1.sem);
        Log.notice("Get pump stage: %D (%u)" CR, dtmp, millis() - now);
        now = millis();
        PT_YIELD_UNTIL(pt, millis() - now > 1000);
      }
    }
  }
#endif
  PT_END(pt);
}