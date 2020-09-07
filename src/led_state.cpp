//
// Created by wuyua on 2020-09-06.
//
#include "led_state.h"
LEDState led;
void LEDState::begin() {
  pinMode(LED_PIN, OUTPUT);
  normal();
}
void LEDState::normal() { jl.FadeOn(1000).FadeOff(1000).Forever(); }

void LEDState::error() { jl.Blink(200, 200).Forever(); }

void LEDState::update() { jl.Update(); }

LEDState::LEDState() : jl(JLed(LED_PIN).LowActive()) { normal(); }
