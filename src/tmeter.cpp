//
// Created by wuyua on 2020-09-06.
//

#include "tmeter.h"

void TMeter::begin() {
  dt.begin();
  dt.setResolution(DS18_RESOLUTION);
}
bool TMeter::ok() { return true; }
char TMeter::get_temperature(TMeterPT& pt, float& temperature) {
  PT_BEGIN(&pt.pt);
  if (ok()) {
    pt.time = millis();
    dt.requestTemperaturesByIndex(0);
    PT_YIELD_UNTIL(&pt.pt,
                   millis() - pt.time > 750 / (1 << (12 - DS18_RESOLUTION)));
    const float t = dt.getTempCByIndex(0);
    if (t == DEVICE_DISCONNECTED_C) {
      LOGE("DS18 disconnected.");
      temperature = t;
      led.error();
    } else {
      temperature = t;
      LOGVF("Temperature = %f", t);
    }
    PT_SEM_SIGNAL(&pt.pt, &pt.sem);
  }
  PT_END(&pt.pt);
}
TMeter::TMeter() : ow(PIN_OW), dt(&ow) {}
