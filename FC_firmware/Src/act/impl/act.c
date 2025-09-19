#include "act/act.h"
#include "err/err.h"
#include "llc/llc.h"
#include "log/log.h"

#include "arm_math.h"

#include <stdbool.h>
#include <stdint.h>

static volatile bool _act_armed = false;

typedef struct [[nodiscard]] act_Motor {
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

        HAL_TIM_PWM_Start(timer, channel);
        __HAL_TIM_SET_COMPARE(timer, channel, act_PWM_MIN + (0 * act_PWM_RANGE / 100));
    }
}

static void act_setMotorSpeed(uint8_t idx, uint16_t percent) {
    err_assert(idx < act_MOTOR_COUNT);

    act_Motor m = _act_motors[idx];
    __HAL_TIM_SET_COMPARE(m.timer, m.channel, act_PWM_MIN + (percent * act_PWM_RANGE / 100));

    // log_debug("beallitva %d. motor\r\n", motor.number);
}

void act_output(llc_ThrustVec in) {
    err_assert(act_MOTOR_COUNT == 4);

    if (!_act_armed) {
        act_disarm();
        return;
    }

    float speed[act_MOTOR_COUNT] = {
        in.thrust + in.yaw + in.pitch + in.roll, //
        in.thrust - in.yaw + in.pitch - in.roll, //
        in.thrust + in.yaw - in.pitch + in.roll, //
        in.thrust - in.yaw - in.pitch - in.roll  //
    };

    float tmp[act_MOTOR_COUNT];

    // scale from speed to tmp
    arm_scale_f32(speed, act_PWM_RANGE, tmp, act_MOTOR_COUNT);
    // clip from tmp to speed
    arm_clip_f32(tmp, speed, act_PWM_MIN, act_PWM_MAX, act_MOTOR_COUNT);

    for (int i = 0; i < act_MOTOR_COUNT; i++)
        act_setMotorSpeed(i, (uint16_t) speed[i]);

    // log_debug("1:%d 2:%d 3:%d 4:%d\n", speed[0], speed[1], speed[2], speed[3]);
}

void act_arm(void) {
    err_assert(!_act_armed);

    _act_armed = true;
}

void act_disarm(void) {
    for (int i = 0; i < act_MOTOR_COUNT; i++)
        act_setMotorSpeed(i, 0);

    // we assert later, to make sure that the motors get turned off no matter what
    bool was_armed = _act_armed;
    _act_armed = false;
    err_assert(!was_armed);
}
