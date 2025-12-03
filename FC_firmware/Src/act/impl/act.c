#include "act/act.h"
#include "err/err.h"
#include "llc/llc.h"
#include "log/log.h"

#include "arm_math.h"

#include <stdbool.h>
#include <stdint.h>

#define _act_PWM_MIN 1000
#define _act_PWM_MAX 2000

#define _act_PWM_RANGE (_act_PWM_MAX - _act_PWM_MIN)

static volatile bool _act_armed = false;

typedef struct [[nodiscard]] _act_Motor {
    TIM_HandleTypeDef* timer;
    uint32_t channel;
} _act_Motor;

static _act_Motor _act_motors[4];

static void _act_setMotorSpeed(uint8_t idx, uint16_t value) {
    err_assert(idx < act_MOTOR_COUNT);
    err_assert(_act_PWM_MIN <= value && value <= _act_PWM_MAX);

    _act_Motor m = _act_motors[idx];
    __HAL_TIM_SET_COMPARE(m.timer, m.channel, value);
}

void act_init(TIM_HandleTypeDef* timers[act_MOTOR_COUNT], uint32_t channels[act_MOTOR_COUNT]) {
    log_debug("Initializing act...");

    for (int i = 0; i < act_MOTOR_COUNT; i++) {
        _act_motors[i] = (_act_Motor) {
            .channel = channels[i],
            .timer = timers[i],
        };
    }
}

act_FinalSignalTelemetry act_output(llc_ThrustVec in) {
    err_assert(act_MOTOR_COUNT == 4);
    err_assert(_act_armed);

    // FIXME: motor matrix
    float tmp[act_MOTOR_COUNT] = {
        in.thrust + 0.1f * (in.yaw + in.pitch - in.roll), //
        in.thrust - 0.1f * (in.yaw + in.pitch + in.roll), //
        in.thrust + 0.1f * (in.yaw - in.pitch + in.roll), //
        in.thrust - 0.1f * (in.yaw - in.pitch - in.roll)  //
    };

    float speed[act_MOTOR_COUNT];

    // scale in place
    arm_scale_f32(tmp, _act_PWM_RANGE, tmp, act_MOTOR_COUNT);

    // offset in place
    arm_offset_f32(tmp, _act_PWM_MIN, tmp, act_MOTOR_COUNT);

    // clip from speed to tmp
    arm_clip_f32(tmp, speed, _act_PWM_MIN, _act_PWM_MAX, act_MOTOR_COUNT);

    for (int i = 0; i < act_MOTOR_COUNT; i++)
        _act_setMotorSpeed(i, (uint16_t) speed[i]);

    act_FinalSignalTelemetry out;
    for (int i = 0; i < act_MOTOR_COUNT; i++)
        out.motorSignals[i] = (uint16_t) speed[i];

    return out;
}

void act_arm(void) {
    bool was_armed = _act_armed;

    for (int i = 0; i < act_MOTOR_COUNT; i++) {
        _act_Motor m = _act_motors[i];
        _act_setMotorSpeed(i, _act_PWM_MIN);
        HAL_TIM_PWM_Start(m.timer, m.channel);
    }

    _act_armed = true;
    err_assert(!was_armed);
    log_info("Armed");
}

void act_disarm(void) {
    for (int i = 0; i < act_MOTOR_COUNT; i++) {
        _act_Motor m = _act_motors[i];
        HAL_TIM_PWM_Stop(m.timer, m.channel);
    }

    // we assert later, to make sure that the motors get turned off no matter what
    bool was_armed = _act_armed;
    _act_armed = false;
    err_assert(was_armed);

    log_info("Disarmed");
}

bool act_isArmed(void) {
    return _act_armed;
}
