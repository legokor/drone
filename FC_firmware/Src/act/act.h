#ifndef ACT_H
#define ACT_H

#include "llc/llc.h"

#include "tim.h"

#include <stdint.h>

#define act_MOTOR_COUNT 4

void act_init(TIM_HandleTypeDef* timers[act_MOTOR_COUNT], uint32_t channels[act_MOTOR_COUNT]);

void act_arm(void);
bool act_isArmed(void);
void act_disarm(void);

typedef struct act_FinalSignalTelemetry {
    float motorSignals[act_MOTOR_COUNT];
} act_FinalSignalTelemetry;

act_FinalSignalTelemetry act_output(llc_ThrustVec tv);

#endif // ACT_H
