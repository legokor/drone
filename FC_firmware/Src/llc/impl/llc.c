#include "llc/llc.h"
#include "dsp/dsp.h"
#include "err/err.h"
#include "imu/imu.h"
#include "log/log.h"

#include "config.h"

#include "arm_math.h"

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

    _llcRollPid.Kp = _llcRollPidConsts[0] / config_ACT_FREQ;
    _llcRollPid.Ki = _llcRollPidConsts[1] / config_ACT_FREQ;
    _llcRollPid.Kd = _llcRollPidConsts[2] / config_ACT_FREQ;
    arm_pid_init_f32(&_llcRollPid, 1);

    _llcPitchPid.Kp = _llcPitchPidConsts[0] / config_ACT_FREQ;
    _llcPitchPid.Ki = _llcPitchPidConsts[1] / config_ACT_FREQ;
    _llcPitchPid.Kd = _llcPitchPidConsts[2] / config_ACT_FREQ;
    arm_pid_init_f32(&_llcPitchPid, 1);

    _llcYawPid.Kp = _llcYawPidConsts[0] / config_ACT_FREQ;
    _llcYawPid.Ki = _llcYawPidConsts[1] / config_ACT_FREQ;
    _llcYawPid.Kd = _llcYawPidConsts[2] / config_ACT_FREQ;
    arm_pid_init_f32(&_llcYawPid, 1);
}

llc_ThrustVec llc_update(llc_ThrustVec ref) {
    imu_Vec3 pv = dsp_getOutAng();

    llc_ThrustVec out;
    out.roll = arm_pid_f32(&_llcRollPid, ref.roll - pv.roll);
    out.pitch = arm_pid_f32(&_llcPitchPid, ref.pitch - pv.pitch);
    out.yaw = arm_pid_f32(&_llcYawPid, ref.yaw - pv.yaw);

    // tilt compensation
    float tr = tanf(pv.roll), tp = tanf(pv.pitch);
    float f = tr * tr + tp * tp;

    // can't happen: err_tryFatal(f > 0, "Square root of negative number in llc tilt compensation");
    float sq;
    arm_sqrt_f32(f, &sq);

    float inclanation = atanf(sq);

    out.thrust = ref.thrust / arm_cos_f32(inclanation);

    return out;
}
