//
// Created by wuyua on 2020-09-06.
//

#ifndef JULABO_FP50_DS18B20_CONTROL_DEBUG_H
#define JULABO_FP50_DS18B20_CONTROL_DEBUG_H

#ifndef LOGGING_DISABLE_VERBOSE
#define LOGVF(fmt, ...) LOGF("V ", fmt, __VA_ARGS__)
#define LOGV(fmt) LOG("V ", fmt)
#else
#define LOGVF(fmt, ...)
#define LOGV(fmt)
#endif

#define LOGN(fmt) LOG("N ", fmt)
#define LOGNF(fmt, ...) LOGF("N ", fmt, __VA_ARGS__)

#define LOGEF(fmt, ...) LOGF("E ", fmt, __VA_ARGS__)
#define LOGE(fmt) LOG("E ", fmt)

#ifndef NDEBUG
void debug_write_helper(const char* fmt, ...);

#define LOGF(level, fmt, ...) debug_write_helper(level fmt "\n", ##__VA_ARGS__);
#define LOG(level, str) debug_write_helper(level str "\n");
#else
#define LOGF(level, fmt, ...)
#define LOG(level, str)
#endif
#endif  // JULABO_FP50_DS18B20_CONTROL_DEBUG_H
