//
// Created by wuyuanyi on 2020-09-06.
//

#ifndef JULABO_FP50_DS18B20_CONTROL_FP50_TEST_H
#define JULABO_FP50_DS18B20_CONTROL_FP50_TEST_H
#include "FP50.h"
#include "pt/pt.h"

PT_THREAD(run_fp50_tests(struct pt* pt, FP50& serial));

#endif  // JULABO_FP50_DS18B20_CONTROL_FP50_TEST_H
