#include "dsp/dsp.h"
#include "imu/imu.h"
#include "log/log.h"

#include "arm_math.h"
#include "stm32f4xx_hal.h"

#include <math.h>

// the definitions for the inputs / outputs
#define _dsp_DATA_IMPL(type, name)  \
    static type _dsp_##name;        \
    type dsp_get##name(void) {      \
        return _dsp_##name;         \
    }                               \
                                    \
    void dsp_set##name(type name) { \
        _dsp_##name = name;         \
    }

_dsp_DATA

#undef _dsp_DATA_IMPL

static uint32_t _dsp_lastRun = 0;

// static imu_Vec3 _dsp_angleGyr = { 0 };

static float _dsp_alpha = 0.97;

void dsp_init(void) {
    log_debug("Initializing dsp...");

    _dsp_lastRun = HAL_GetTick();
}

void dsp_update(void) {
    // TODO: use a timer for usec resolution
    uint32_t now = HAL_GetTick();
    float dt = 0.001 * (now - _dsp_lastRun);
    _dsp_lastRun = now;

    imu_Vec3 inAcc = dsp_getInAcc();
    imu_Vec3 inGyr = dsp_getInGyr();

    static imu_Vec3 _dsp_gyrIntegral = { 0 };

    // _dsp_gyrIntegral.roll += inGyr.roll * dt;
    // _dsp_gyrIntegral.pitch += inGyr.pitch * dt;
    // _dsp_gyrIntegral.yaw += inGyr.yaw * dt;
    imu_Vec3 tmp;
    arm_add_f32(_dsp_gyrIntegral.arr, inGyr.arr, tmp.arr, 3);
    arm_scale_f32(tmp.arr, dt, _dsp_gyrIntegral.arr, 3);

    float rollA = atan2f(inAcc.x, inAcc.z);
    float pitchA = atan2f(inAcc.y, inAcc.z);

    static float _dps_rollComp = 0, _dsp_pitchComp = 0;

    _dps_rollComp = rollA * (1 - _dsp_alpha) + _dsp_alpha * (_dps_rollComp + inGyr.y * dt);
    _dsp_pitchComp = pitchA * (1 - _dsp_alpha) + _dsp_alpha * (_dsp_pitchComp + inGyr.x * dt);

    dsp_setOutAng((imu_Vec3) { .roll = _dps_rollComp, .pitch = _dsp_pitchComp, .yaw = _dsp_gyrIntegral.yaw });
}
