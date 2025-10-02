#include "llc/llc.h"
#include "dsp/dsp.h"
#include "err/err.h"
#include "imu/imu.h"
#include "log/log.h"

#include "config.h"

#include "arm_math.h"

static arm_pid_instance_f32 _llc_pid_roll, _llc_pid_pitch, _llc_pid_yaw;

static float _llc_roll_pid_consts[3] = { CONFIG_LLC_ROLL_PID_CONSTS };
static float _llc_pitch_pid_consts[3] = { CONFIG_LLC_PITCH_PID_CONSTS };
static float _llc_yaw_pid_consts[3] = { CONFIG_LLC_YAW_PID_CONSTS };

static void _llc_reinit_pids() {
    _llc_pid_roll.Kp = _llc_roll_pid_consts[0] / CONFIG_ACT_FREQ;
    _llc_pid_roll.Ki = _llc_roll_pid_consts[1] / CONFIG_ACT_FREQ;
    _llc_pid_roll.Kd = _llc_roll_pid_consts[2] / CONFIG_ACT_FREQ;
    arm_pid_init_f32(&_llc_pid_roll, 0);

    _llc_pid_pitch.Kp = _llc_pitch_pid_consts[0] / CONFIG_ACT_FREQ;
    _llc_pid_pitch.Ki = _llc_pitch_pid_consts[1] / CONFIG_ACT_FREQ;
    _llc_pid_pitch.Kd = _llc_pitch_pid_consts[2] / CONFIG_ACT_FREQ;
    arm_pid_init_f32(&_llc_pid_pitch, 0);

    _llc_pid_yaw.Kp = _llc_yaw_pid_consts[0] / CONFIG_ACT_FREQ;
    _llc_pid_yaw.Ki = _llc_yaw_pid_consts[1] / CONFIG_ACT_FREQ;
    _llc_pid_yaw.Kd = _llc_yaw_pid_consts[2] / CONFIG_ACT_FREQ;
    arm_pid_init_f32(&_llc_pid_yaw, 0);
}

void llc_set_consts(float roll_p,
                    float roll_i,
                    float roll_d,
                    float pitch_p,
                    float pitch_i,
                    float pitch_d,
                    float yaw_p,
                    float yaw_i,
                    float yaw_d) {

    _llc_roll_pid_consts[0] = roll_p;
    _llc_roll_pid_consts[1] = roll_i;
    _llc_roll_pid_consts[2] = roll_d;

    _llc_pitch_pid_consts[0] = pitch_p;
    _llc_pitch_pid_consts[1] = pitch_i;
    _llc_pitch_pid_consts[2] = pitch_d;

    _llc_yaw_pid_consts[0] = yaw_p;
    _llc_yaw_pid_consts[1] = yaw_i;
    _llc_yaw_pid_consts[2] = yaw_d;

    _llc_reinit_pids();
}

void llc_init(void) {
    log_debug("Initializing llc...");

    // TODO: tune PID
    _llc_reinit_pids();
}

llc_ThrustVec llc_update(llc_ThrustVec ref) {
    imu_Vec3 pv = dsp_getOutAng();

    llc_ThrustVec out;
    out.roll = arm_pid_f32(&_llc_pid_roll, ref.roll - pv.roll);
    out.pitch = arm_pid_f32(&_llc_pid_pitch, ref.pitch - pv.pitch);
    out.yaw = arm_pid_f32(&_llc_pid_yaw, ref.yaw - pv.yaw);

    // tilt compensation
    float tr = tanf(pv.roll), tp = tanf(pv.pitch);
    float sq;

    if (arm_sqrt_f32(tr * tr + tp * tp, &sq) != ARM_MATH_SUCCESS) {
        err_ignorable("Square root of negative number in dsp tilt compensation");
        err_todo("???");
    }

    float inclanation = atanf(sq);

    out.thrust = ref.thrust / arm_cos_f32(inclanation);

    return out;
}
