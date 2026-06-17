#include "llc/llc.h"
#include "dsp/dsp.h"
#include "err/err.h"
#include "imu/imu.h"
#include "log/log.h"

#include "config.h"

#include "arm_math.h"
#include "utils/utils.h"

#include <math.h>

static arm_pid_instance_f32 _llcRollPid, _llcPitchPid, _llcYawPid;

static float _llcRollPidConsts[3] = { config_LLC_ROLL_PID_CONSTS };
static float _llcPitchPidConsts[3] = { config_LLC_PITCH_PID_CONSTS };
static float _llcYawPidConsts[3] = { config_LLC_YAW_PID_CONSTS };

#define __llc_PID_HELPER(axis, const, idx)     \
    void llc_set##axis##const(float val) {     \
        _llc##axis##PidConsts[idx] = val;      \
        arm_pid_init_f32(&_llc##axis##Pid, 0); \
    }

__llc_PID_HELPER(Roll, P, 0)
__llc_PID_HELPER(Roll, I, 1)
__llc_PID_HELPER(Roll, D, 2)
__llc_PID_HELPER(Pitch, P, 0)
__llc_PID_HELPER(Pitch, I, 1)
__llc_PID_HELPER(Pitch, D, 2)
__llc_PID_HELPER(Yaw, P, 0)
__llc_PID_HELPER(Yaw, I, 1)
__llc_PID_HELPER(Yaw, D, 2)

#undef __llc_PID_HELPER

void llc_init(void) {
    log_debug("Initializing llc...");

    // TODO: tune PID

    // _llcRollPid.Kp = _llcRollPidConsts[0] / (1000.0f / config_ACT_LOOP_FREQ);
    // _llcRollPid.Ki = _llcRollPidConsts[1] / (1000.0f / config_ACT_LOOP_FREQ);
    // _llcRollPid.Kd = _llcRollPidConsts[2] / (1000.0f / config_ACT_LOOP_FREQ);
    // arm_pid_init_f32(&_llcRollPid, 1);

    // _llcPitchPid.Kp = _llcPitchPidConsts[0] / (1000.0f / config_ACT_LOOP_FREQ);
    // _llcPitchPid.Ki = _llcPitchPidConsts[1] / (1000.0f / config_ACT_LOOP_FREQ);
    // _llcPitchPid.Kd = _llcPitchPidConsts[2] / (1000.0f / config_ACT_LOOP_FREQ);
    // arm_pid_init_f32(&_llcPitchPid, 1);

    // _llcYawPid.Kp = _llcYawPidConsts[0] / (1000.0f / config_ACT_LOOP_FREQ);
    // _llcYawPid.Ki = _llcYawPidConsts[1] / (1000.0f / config_ACT_LOOP_FREQ);
    // _llcYawPid.Kd = _llcYawPidConsts[2] / (1000.0f / config_ACT_LOOP_FREQ);
    // arm_pid_init_f32(&_llcYawPid, 1);
}

// in:
//   - desired roll, pitch angle to gravity (earth up)
//   - yaw angular speed

// out:

float PID(float sp, float pv, float t, float consts[3], float* integral, float* last_err) {
    float err = sp - pv;

    *integral = utils_clampF((*integral) + err * t, -0.03, 0.03);

    float d = (err - *last_err) / t;
    *last_err = err;

    return consts[0] * err + consts[1] * *integral + consts[2] * d;
}

static float _llcRollIntegral = 0;
static float _llcRollLastErr = 0;
static float _llcPitchIntegral = 0;
static float _llcPitchLastErr = 0;
static float _llcYawIntegral = 0;
static float _llcYawLastErr = 0;

llc_ThrustVec llc_update(llc_ThrustVec ref) {
    return ref;

    imu_Vec3 pv = dsp_getOutAng();

    llc_ThrustVec out;
    // out.roll = arm_pid_f32(&_llcRollPid, ref.roll - pv.roll);
    // out.pitch = arm_pid_f32(&_llcPitchPid, ref.pitch - pv.pitch);
    // out.yaw = arm_pid_f32(&_llcYawPid, ref.yaw - pv.yaw);

    const float dt = 1000.0f / config_ACT_LOOP_FREQ;

    out.roll = PID(ref.roll, pv.roll, dt, _llcRollPidConsts, &_llcRollIntegral, &_llcRollLastErr);
    out.pitch = PID(ref.pitch, pv.pitch, dt, _llcPitchPidConsts, &_llcPitchIntegral, &_llcPitchLastErr);
    out.yaw = PID(ref.yaw, pv.yaw, dt, _llcYawPidConsts, &_llcYawIntegral, &_llcYawLastErr);

    // tilt compensation
    float tr = tanf(pv.roll), tp = tanf(pv.pitch);
    float f = tr * tr + tp * tp;

    // can't happen: err_tryFatal(f > 0, "Square root of negative number in llc tilt compensation");
    float sq = sqrtf(f);

    float inclanation = atanf(sq);

    out.thrust = ref.thrust / cosf(inclanation);

    return out;
}
