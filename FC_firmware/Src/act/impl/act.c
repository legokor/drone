#include "act/act.h"
#include "act/impl/dshot.h"
#include "act/impl/pwm.h"
#include "config.h"
#include "err/err.h"
#include "llc/llc.h"
#include "log/log.h"

#include <stdbool.h>
#include <stdint.h>

static volatile bool _act_armed = false;

void act_init(TIM_HandleTypeDef* timers[act_MOTOR_COUNT], uint32_t channels[act_MOTOR_COUNT]) {
    log_debug("Initializing act...");

#if config_ACT_MODE == config_ACT_MODE_PWM
    act_pwm_init(timers, channels);
#elif config_ACT_MODE == config_ACT_MODE_DSHOT
    dshot_init(timers, channels);
#endif
}

act_FinalSignalTelemetry act_output(llc_ThrustVec in) {
    err_assert(act_MOTOR_COUNT == 4);
    err_assert(_act_armed);

    // FIXME: motor matrix
    float speeds[act_MOTOR_COUNT] = {
        in.thrust + in.roll + in.pitch + in.yaw, //
        in.thrust + in.roll - in.pitch - in.yaw, //
        in.thrust - in.roll + in.pitch - in.yaw, //
        in.thrust - in.roll - in.pitch + in.yaw  //
    };

#if config_ACT_MODE == config_ACT_MODE_PWM
    act_pwm_output(speeds);
#elif config_ACT_MODE == config_ACT_MODE_DSHOT
    dshot_outputSpeed(speeds);
#endif

    act_FinalSignalTelemetry out;
    for (int i = 0; i < act_MOTOR_COUNT; i++) {
        out.motorSignals[i] = speeds[i];
    }

    return out;
}

void act_arm(void) {
    bool was_armed = _act_armed;

#if config_ACT_MODE == config_ACT_MODE_PWM
    act_pwm_arm();
#elif config_ACT_MODE == config_ACT_MODE_DSHOT
    dshot_disarm();
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
    dshot_arm();
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
