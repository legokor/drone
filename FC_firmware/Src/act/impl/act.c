#include "act/act.h"
#include "err/err.h"
#include "llc/llc.h"
#include "log/log.h"

#include "config.h"
#include "sys/time.h"

#if config_ACT_MODE == config_ACT_MODE_PWM
#include "act/impl/pwm.h"
#elif config_ACT_MODE == config_ACT_MODE_DSHOT
#include "act/impl/dshot.h"
#endif

#include <stdbool.h>
#include <stdint.h>

static volatile bool _act_armed = false;

void act_init(TIM_HandleTypeDef* const timers[act_MOTOR_COUNT], const uint32_t channels[act_MOTOR_COUNT]) {
    log_debug("Initializing act...");

#if config_ACT_MODE == config_ACT_MODE_PWM
    act_pwm_init(timers, channels);
#elif config_ACT_MODE == config_ACT_MODE_DSHOT
    act_dshot_init(timers, channels);
#endif
}

act_FinalSignalTelemetry act_output(llc_ThrustVec in) {
    err_assert(act_MOTOR_COUNT == 4);
    err_assert(_act_armed);

    // FIXME: motor matrix
    float speeds[act_MOTOR_COUNT] = {
        utils_clamp(in.thrust + in.roll + in.pitch + in.yaw, 0, 1), //
        utils_clamp(in.thrust + in.roll - in.pitch - in.yaw, 0, 1), //
        utils_clamp(in.thrust - in.roll + in.pitch - in.yaw, 0, 1), //
        utils_clamp(in.thrust - in.roll - in.pitch + in.yaw, 0, 1)  //
    };

    act_FinalSignalTelemetry out;

#if config_ACT_MODE == config_ACT_MODE_PWM
    act_pwm_output(speeds, &out);
#elif config_ACT_MODE == config_ACT_MODE_DSHOT
    act_dshot_outputSpeed(speeds, &out);
#endif

    for (int i = 0; i < act_MOTOR_COUNT; i++)
        out.motorSignals[i] = speeds[i];

    return out;
}

static sys_TimeUS _act_armedT = 0;

sys_TimeUS act_getArmTime(void) {
    return _act_armedT;
}

void act_arm(void) {
    _act_armedT = sys_time();
    bool was_armed = _act_armed;

#if config_ACT_MODE == config_ACT_MODE_PWM
    act_pwm_arm();
#elif config_ACT_MODE == config_ACT_MODE_DSHOT
    act_dshot_arm();
#endif

    _act_armed = true;
    err_assert(!was_armed);
    log_info("armed");
}

void act_disarm(void) {
    // TODO: pwm min instead of stopping?

#if config_ACT_MODE == config_ACT_MODE_PWM
    act_pwm_disarm();
#elif config_ACT_MODE == config_ACT_MODE_DSHOT
    act_dshot_disarm();
#endif

    // we assert later, to make sure that the motors get turned off no matter what
    bool was_armed = _act_armed;
    _act_armed = false;
    err_assert(was_armed);

    log_info("disarmed");
}

bool act_isArmed(void) {
    return _act_armed;
}
