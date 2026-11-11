#include "guide/guide.h"
#include "act/act.h"
#include "err/err.h"
#include "llc/llc.h"
#include "log/log.h"
#include "rc/rc.h"
#include "sys/sys.h"
#include "sys/time.h"
#include "utils/utils.h"

#include "config.h"

void guide_init(void) {
    log_debug("Initializing guide...");
}

#define __guide_SBUS_RANGE_MIN 0
#define __guide_SBUS_RANGE_MAX 2048
#define __guide_SBUS_RANGE __guide_SBUS_RANGE_MIN, __guide_SBUS_RANGE_MAX

#define __guide_ROLL_MAP_ARGS config_RC_ROLL_CALIB_RANGE, config_RC_ROLL_PITCH_YAW_RANGE
#define __guide_PITCH_MAP_ARGS config_RC_PITCH_CALIB_RANGE, config_RC_ROLL_PITCH_YAW_RANGE
#define __guide_YAW_MAP_ARGS config_RC_YAW_CALIB_RANGE, config_RC_ROLL_PITCH_YAW_RANGE
#define __guide_THRUST_MAP_ARGS config_RC_THRUST_CALIB_RANGE, config_RC_THRUST_RANGE

//
// TODO: remove
#define __guide_TEST_PROGRAM_SWITCH_CHANNEL 8
#define __guide_TEST_PROGRAM_LENGTH_MS 30000
#define __guide_TEST_PROGRAM_MAX_THRUST 1.0f

static sys_TimeUS __guide_start_t = 0;
static bool __guide_testProgramActive = false;

// pyramid
static float __guide_testThrustProgram(void) {
    const float half = __guide_TEST_PROGRAM_LENGTH_MS / 2.0f;

    float t = sys_timeDiff(__guide_start_t, sys_time()) * 0.001f;

    if (t < half) {
        return t / half * __guide_TEST_PROGRAM_MAX_THRUST;
    } else if (t <= __guide_TEST_PROGRAM_LENGTH_MS) {
        return (1.0f - (t - half) / half) * __guide_TEST_PROGRAM_MAX_THRUST;
    } else {
        __guide_testProgramActive = false;
        return 0.0f;
    }
}
//

static llc_ThrustVec _guide_rcMode() {
    rc_RxPackage d;

    // TODO: fatal?
    bool gotRC = rc_getData(&sys_rcInstance, &d);
    err_tryIgnorable(gotRC, "Failed to get rc data");

    llc_ThrustVec ref;
    if (gotRC) {

        if (__guide_testProgramActive) {
            ref = (llc_ThrustVec){
                .roll = 0.0f,
                .pitch = 0.0f,
                .yaw = 0.0f,
                .thrust = utils_clampF(__guide_testThrustProgram(), 0.0f, __guide_TEST_PROGRAM_MAX_THRUST),
            };
        } else {

            // TODO: correct channels
            ref = (llc_ThrustVec){
                .roll = utils_mapF(d.channels[config_RC_CHAN_ROLL], __guide_ROLL_MAP_ARGS),
                .pitch = utils_mapF(d.channels[config_RC_CHAN_PITCH], __guide_PITCH_MAP_ARGS),
                .yaw = utils_mapF(d.channels[config_RC_CHAN_YAW], __guide_YAW_MAP_ARGS),
                .thrust = utils_mapF(d.channels[config_RC_CHAN_THRUST], __guide_THRUST_MAP_ARGS),
            };

            // FIXME: max thrust clamped
            // if (ref.thrust > 0.80f)
            //     ref.thrust = config_RC_THRUST_MAX;

            ref.roll = 0;  // utils_clampF(ref.roll, config_RC_ROLL_PITCH_YAW_RANGE);
            ref.pitch = 0; // utils_clampF(ref.pitch, config_RC_ROLL_PITCH_YAW_RANGE);
            ref.yaw = 0;   // utils_clampF(ref.yaw, config_RC_ROLL_PITCH_YAW_RANGE);
            ref.thrust = utils_clampF(ref.thrust, config_RC_THRUST_RANGE);
        }

        // FIXME: arm value
        if (act_isArmed()) {
            if (!(d.channels[config_RC_CHAN_ARM] < config_RC_ARM_THRESHOLD)) {
                act_disarm();
            } else if (!__guide_testProgramActive && d.channels[__guide_TEST_PROGRAM_SWITCH_CHANNEL] > 1500) {
                __guide_testProgramActive = true;
                __guide_start_t = sys_time();
            }
        } else {
            if (d.channels[config_RC_CHAN_ARM] < config_RC_ARM_THRESHOLD) {
                act_arm();
                __guide_testProgramActive = false;
            }
        }
    } else {
        // TODO: hover
        ref = (llc_ThrustVec){
            .roll = 0.0f,
            .pitch = 0.0f,
            .yaw = 0.0f,
            .thrust = 0.0f,
        };
    }

    return ref;
}

llc_ThrustVec guide_getRef(ctrl_Mode mode) {
    switch (mode) {
        case ctrl_Idle: return (llc_ThrustVec){ 0 };

        case ctrl_RC: return _guide_rcMode();

        case ctrl_SoftLanding: err_todo(); break;
        case ctrl_GPS_Waypoint: err_todo(); break;
    }

    err_unreachable();
}
