//
// Created by wuyua on 2020-09-06.
//

#ifndef JULABO_FP50_DS18B20_CONTROL_TMETER_H
#define JULABO_FP50_DS18B20_CONTROL_TMETER_H
#include "DallasTemperature.h"
#include "debug.h"
#include "pt/asyncpt.h"
#include "led_state.h"
#include "pt/pt.h"
#define PIN_OW 4
#define DS18_RESOLUTION 12

class TMeterPT : public AsyncPT {
 public:
  ulong time{millis()};
};
class TMeter {
 private:
  OneWire ow;
  DallasTemperature dt;
  DeviceAddress addr{};
  int devCount{};

 public:
  TMeter();
  void begin();
  PT_THREAD(get_temperature(TMeterPT& pt, float& temperature));
  bool ok();
};
#endif  // JULABO_FP50_DS18B20_CONTROL_TMETER_H
