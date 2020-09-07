//
// Created by wuyua on 2020-09-06.
//

#ifndef JULABO_FP50_DS18B20_CONTROL_ASYNCPT_H
#define JULABO_FP50_DS18B20_CONTROL_ASYNCPT_H
#include "pt/pt.h"
#include "pt/pt-sem.h"
struct AsyncPT {
  struct pt pt {};
  struct pt_sem sem {};
  void init() {
    PT_INIT(&pt);
    PT_SEM_INIT(&sem, 0);
  }
  AsyncPT() { init(); }
};
#endif  // JULABO_FP50_DS18B20_CONTROL_ASYNCPT_H
