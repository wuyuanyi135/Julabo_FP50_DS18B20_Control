//
// Created by wuyua on 2020-09-06.
//

#ifndef JULABO_FP50_DS18B20_CONTROL_DEBUG_H
#define JULABO_FP50_DS18B20_CONTROL_DEBUG_H

#ifndef LOGGING_DISABLE_VERBOSE
#define LOGVF(fmt, ...) LOGF(false, "V ", fmt, __VA_ARGS__)
#define LOGV(fmt) LOG(false, \
                      "V ", fmt)
#else
#define LOGVF(fmt, ...)
#define LOGV(fmt)
#endif

#define LOGN(fmt) LOG(true, "N ", fmt)
#define LOGNF(fmt, ...) LOGF(true, "N ", fmt, __VA_ARGS__)

#define LOGEF(fmt, ...) LOGF(true, "E ", fmt, __VA_ARGS__)
#define LOGE(fmt) LOG(true, "E ", fmt)

#ifndef NDEBUG
void debug_write_helper(bool remote, const char* fmt, ...);

#define LOGF(remote, level, fmt, ...) debug_write_helper(remote, level fmt "\n", __VA_ARGS__);
#define LOG(remote, level, str) debug_write_helper(remote, level str "\n");
#else
#define LOGF(level, fmt, ...)
#define LOG(level, str)
#endif
#endif  // JULABO_FP50_DS18B20_CONTROL_DEBUG_H
