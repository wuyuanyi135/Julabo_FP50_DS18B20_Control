//
// Created by wuyua on 2020-09-06.
//

#ifndef JULABO_FP50_DS18B20_CONTROL_LED_STATE_H
#define JULABO_FP50_DS18B20_CONTROL_LED_STATE_H
#include "Arduino.h"
#include "jled.h"
#define LED_PIN 2

class LEDState {
 public:
  LEDState();
  void begin();
  void normal();
  void error();

  void update();
 private:
  JLed jl;
  int errorCount{0};
};

extern LEDState led;
#endif  // JULABO_FP50_DS18B20_CONTROL_LED_STATE_H
