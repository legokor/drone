#ifndef TEL_H
#define TEL_H

#include "imu/imu.h"
#include "llc/llc.h"

typedef void (*tel_WriterFn)(const void*, size_t);

void tel_init(void);
void tel_addSource(tel_WriterFn source);

#define tel_TOPICS                                                   \
    tel_DEFINE_TOPIC_X(0, char*)               /* logs */            \
        tel_DEFINE_TOPIC_X(1, float)           /* battery voltage */ \
        tel_DEFINE_TOPIC_X(2, imu_Vec3, float) /* imu, altitude */   \
        tel_DEFINE_TOPIC_X(3, llc_ThrustVec)   /* RC input */        \
        tel_DEFINE_TOPIC_X(4, llc_ThrustVec)   /* PID outputs */

#define tel_DEFINE_TOPIC_X(n, ...) void tel_writeToTopic##n(__VA_ARGS__);
tel_TOPICS
#undef tel_DEFINE_TOPIC_X

#define tel_DEFINE_TOPIC_X(n, ...) \
    do {                           \
    } while (0);

void test(void) {
    tel_TOPICS
}
#undef tel_DEFINE_TOPIC_X

#endif // TEL_H
