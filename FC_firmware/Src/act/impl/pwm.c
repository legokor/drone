
#include "act/act.h"
#include "err/err.h"

#include <stdint.h>

typedef struct [[nodiscard]] _act_pwm_Motor {
    TIM_HandleTypeDef* timer;
    uint32_t channel;
} _act_pwm_Motor;

static _act_pwm_Motor _act_pwm_motors[4];

#define _act_pwm_PWM_MIN 1000
#define _act_pwm_PWM_MAX 2000

#define _act_pwm_PWM_RANGE (_act_pwm_PWM_MAX - _act_pwm_PWM_MIN)

static void _act_pwm_setMotorSpeed(uint8_t idx, uint16_t value) {
    err_assert(idx < act_MOTOR_COUNT);
    err_assert(_act_pwm_PWM_MIN <= value && value <= _act_pwm_PWM_MAX);

    _act_pwm_Motor m = _act_pwm_motors[idx];
    __HAL_TIM_SET_COMPARE(m.timer, m.channel, value);
}

void act_pwm_output(float speed[act_MOTOR_COUNT]) {

    // dshot_output(speed);

    // float speed[act_MOTOR_COUNT];

    // // scale in place
    // arm_scale_f32(tmp, _act_PWM_RANGE, tmp, act_MOTOR_COUNT);

    // // offset in place
    // arm_offset_f32(tmp, _act_PWM_MIN, tmp, act_MOTOR_COUNT);

    // // clip from speed to tmp
    // arm_clip_f32(tmp, speed, _act_PWM_MIN, _act_PWM_MAX, act_MOTOR_COUNT);

    for (int i = 0; i < act_MOTOR_COUNT; i++) {
        uint16_t s = utils_clamp(_act_pwm_PWM_MIN + _act_pwm_PWM_RANGE * speed[0], _act_pwm_PWM_MIN, _act_pwm_PWM_MAX);
        _act_pwm_setMotorSpeed(i, s);
    }
}

void act_pwm_arm(void) {
    for (int i = 0; i < act_MOTOR_COUNT; i++) {
        _act_pwm_setMotorSpeed(i, _act_pwm_PWM_MIN);
#ifdef config_ACT_NO_PWM_ON_DISARM
        _act_Motor m = _act_motors[i];
        HAL_TIM_PWM_Start(m.timer, m.channel);
#endif
    }
}

void act_pwm_disarm(void) {

    for (int i = 0; i < act_MOTOR_COUNT; i++) {
#ifdef config_ACT_NO_PWM_ON_DISARM
        _act_Motor m = _act_motors[i];
        HAL_TIM_PWM_Stop(m.timer, m.channel);
#else
        _act_pwm_setMotorSpeed(i, _act_pwm_PWM_MIN);
#endif
    }
}

void act_pwm_init(TIM_HandleTypeDef* timers[act_MOTOR_COUNT], uint32_t channels[act_MOTOR_COUNT]) {
    for (int i = 0; i < act_MOTOR_COUNT; i++) {
        _act_pwm_motors[i] = (_act_pwm_Motor){
            .channel = channels[i],
            .timer = timers[i],
        };

#ifndef config_ACT_NO_PWM_ON_DISARM
        _act_pwm_setMotorSpeed(i, _act_pwm_PWM_MIN);
        HAL_TIM_PWM_Start(timers[i], channels[i]);
#endif
    }
}
