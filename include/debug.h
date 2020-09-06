//
// Created by wuyua on 2020-09-06.
//

#ifndef JULABO_FP50_DS18B20_CONTROL_DEBUG_H
#define JULABO_FP50_DS18B20_CONTROL_DEBUG_H

#define LOGVF(fmt, ...) LOGF("V ", fmt, __VA_ARGS__)
#define LOGNF(fmt, ...) LOGF("N ", fmt, __VA_ARGS__)
#define LOGEF(fmt, ...) LOGF("E ", fmt, __VA_ARGS__)

#define LOGV(fmt) LOG("V ", fmt)
#define LOGN(fmt) LOG("N ", fmt)
#define LOGE(fmt) LOG("E ", fmt)

#ifndef NDEBUG
#define DEBUG_OUTPUT Serial
#define LOGF(level, fmt, ...) DEBUG_OUTPUT.printf(level fmt "\n", ##__VA_ARGS__); DEBUG_OUTPUT.flush()
#define LOG(level, str) DEBUG_OUTPUT.print(level str "\n"); DEBUG_OUTPUT.flush()
#else
#define LOGF(level, fmt, ...)
#define LOG(level, str)
#endif
#endif  // JULABO_FP50_DS18B20_CONTROL_DEBUG_H
