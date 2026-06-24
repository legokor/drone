#ifndef TIME_H
#define TIME_H

#include <stdint.h>

typedef uint32_t sys_TimeUS;

[[nodiscard]] sys_TimeUS sys_time(void);
[[nodiscard]] sys_TimeUS sys_timeDiff(sys_TimeUS earlier, sys_TimeUS later);

void sys_busyWaitUntil(sys_TimeUS until);
void sys_busyWait(sys_TimeUS t);

[[nodiscard]] uint64_t sys_totalTimeSinceBootUS(void);

#endif // TIME_H
