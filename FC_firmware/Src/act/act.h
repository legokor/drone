#ifndef ACT_H
#define ACT_H

#include "llc/llc.h"

#include "config.h"

#include "sys/time.h"
#include "tim.h"

#include <stdint.h>

#define act_MOTOR_COUNT 4

#if config_ACT_MODE == config_ACT_MODE_PWM
void act_init(TIM_HandleTypeDef* const timers[act_MOTOR_COUNT], const uint32_t channels[act_MOTOR_COUNT]);
#elif config_ACT_MODE == config_ACT_MODE_DSHOT
void act_init(TIM_HandleTypeDef* timer);
#endif

void act_arm(void);
bool act_isArmed(void);
void act_disarm(void);

sys_TimeUS act_getArmTime(void);

typedef struct act_FinalSignalTelemetry {
    float motorSignals[act_MOTOR_COUNT];

#if config_ACT_MODE == config_ACT_MODE_DSHOT
    uint32_t dshotThrottles[act_MOTOR_COUNT];
#elif config_ACT_MODE == config_ACT_MODE_PWM
    uint32_t pwmDuties[act_MOTOR_COUNT];
#endif
} act_FinalSignalTelemetry;

act_FinalSignalTelemetry act_output(llc_ThrustVec tv);

#endif // ACT_H
