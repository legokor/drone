#include "act/act.h"
#include "log/log.h"

#include <stdbool.h>
#include "arm_math.h"

#include <stdbool.h>

static volatile bool _act_armed = false;

typedef struct act_Motor {
    TIM_HandleTypeDef* timer;
    uint32_t channel;
} act_Motor;

static act_Motor _act_motors[4] = { 0 };

void act_init(TIM_HandleTypeDef* timers[act_MOTOR_COUNT], uint32_t channels[act_MOTOR_COUNT]) {
    log_debug("Initializing act...");

    for (uint8_t i = 0; i < act_MOTOR_COUNT; i++) {
        TIM_HandleTypeDef* timer = timers[i];
        uint32_t channel = channels[i];

        _act_motors[i] = (act_Motor) { .channel = channel, .timer = timer };

        __HAL_TIM_SET_COMPARE(timer, channel, act_PWM_MIN + (0 * act_PWM_RANGE / 100));
        HAL_TIM_PWM_Start(timer, channel);
    }
}

static void act_setMotorSpeed(uint8_t idx, uint16_t percent) {
    assert_param(idx < act_MOTOR_COUNT);

    act_Motor m = _act_motors[idx];
    __HAL_TIM_SET_COMPARE(m.timer, m.channel, act_PWM_MIN + (percent * act_PWM_RANGE / 100));

    // log_debug("beallitva %d. motor\r\n", motor.number);
}

void act_setMMX(float thrust, float yaw, float pitch, float roll) {
    assert_param(act_MOTOR_COUNT == 4);

    if (!_act_armed) {
        act_disarm();
        return;
    }

    float speed[act_MOTOR_COUNT] = {
        thrust + yaw + pitch + roll, //
        thrust - yaw + pitch - roll, //
        thrust + yaw - pitch + roll, //
        thrust - yaw - pitch - roll  //
    };

    float tmp[act_MOTOR_COUNT];

    // scale from speed to tmp
    arm_scale_f32(speed, act_PWM_RANGE, tmp, act_MOTOR_COUNT);
    // clip from tmp to speed
    arm_clip_f32(tmp, speed, act_PWM_MIN, act_PWM_MAX, act_MOTOR_COUNT);

    for (int i = 0; i < act_MOTOR_COUNT; i++)
        act_setMotorSpeed(i, speed[i]);

    // log_debug("1:%d 2:%d 3:%d 4:%d\n", speed[0], speed[1], speed[2], speed[3]);
}

void act_arm(void) {
    assert_param(act_Armed == false);

    _act_armed = true;
}

void act_disarm(void) {
    for (int i = 0; i < act_MOTOR_COUNT; i++)
        act_setMotorSpeed(i, 0);

    // we assert later, to make sure that the motors get turned off no matter what
    bool was_armed = _act_armed;
    _act_armed = false;
}
