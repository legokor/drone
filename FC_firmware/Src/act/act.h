#ifndef ACT_H
#define ACT_H

#include "llc/llc.h"

#include "tim.h"

#include <stdint.h>

#define act_MOTOR_COUNT 4

#define act_PWM_MIN 1000
#define act_PWM_MAX 2000

#define act_PWM_RANGE (act_PWM_MAX - act_PWM_MIN)

void act_init(TIM_HandleTypeDef* timers[act_MOTOR_COUNT], uint32_t channels[act_MOTOR_COUNT]);

void act_arm(void);
void act_disarm(void);

void act_output(llc_ThrustVec tv);

#endif // ACT_H
