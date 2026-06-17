#ifndef TIME_H
#define TIME_H

#include <stdint.h>

typedef uint32_t sys_TimeStampUS;

sys_TimeStampUS sys_time(void);
sys_TimeStampUS sys_timeDiff(sys_TimeStampUS earlier, sys_TimeStampUS later);
void sys_sleepUntill(sys_TimeStampUS untill);

uint64_t sys_totalTimeSinceBootUS(void);

#endif // TIME_H
