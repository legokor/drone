#ifndef ACT_PWM_H
#define ACT_PWM_H

#include "act/act.h"

void act_pwm_init(TIM_HandleTypeDef* timers[act_MOTOR_COUNT], uint32_t channels[act_MOTOR_COUNT]);

void act_pwm_arm(void);
void act_pwm_disarm(void);

void act_pwm_output(float speed[act_MOTOR_COUNT]);

#endif // ACT_PWM_H
