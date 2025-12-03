#include "guide/guide.h"
#include "act/act.h"
#include "err/err.h"
#include "llc/llc.h"
#include "log/log.h"
#include "rc/rc.h"
#include "sys/sys.h"
#include "utils/utils.h"

#include "config.h"

void guide_init(void) {
    log_debug("Initializing guide...");
}

#define __guide_SBUS_RANGE_MIN 0
#define __guide_SBUS_RANGE_MAX 2048
#define __guide_SBUS_RANGE __guide_SBUS_RANGE_MIN, __guide_SBUS_RANGE_MAX

// TODO: ranges
#define __guide_ROLL_PITCH_YAW_MAP_ARGS __guide_SBUS_RANGE, -1, 1
#define __guide_THRUST_MAP_ARGS __guide_SBUS_RANGE, 0, 1

static llc_ThrustVec _guide_rcMode() {
    rc_RxPackage d;

    // TODO: fatal?
    bool gotRC = rc_getData(&sys_rcInstance, &d);
    err_tryIgnorable(gotRC, "Failed to get rc data");

    llc_ThrustVec ref;
    if (gotRC) {

        // TODO: correct channels
        ref = (llc_ThrustVec) {
            .roll = utils_mapF(d.channels[config_RC_CHAN_ROLL], __guide_ROLL_PITCH_YAW_MAP_ARGS),
            .pitch = utils_mapF(d.channels[config_RC_CHAN_PITCH], __guide_ROLL_PITCH_YAW_MAP_ARGS),
            .yaw = utils_mapF(d.channels[config_RC_CHAN_YAW], __guide_ROLL_PITCH_YAW_MAP_ARGS),
            .thrust = utils_mapF(d.channels[config_RC_CHAN_THRUST], __guide_THRUST_MAP_ARGS),
        };

        // FIXME: max thrust clamped
#define L 0.5
        ref.roll = utils_clamp(ref.roll, -L, L);
        ref.pitch = utils_clamp(ref.pitch, -L, L);
        ref.yaw = utils_clamp(ref.yaw, -L, L);
        ref.thrust = utils_clamp(ref.thrust, 0, L);
#undef L

        // FIXME: arm value
        if (act_isArmed()) {
            if (d.channels[config_RC_CHAN_ARM] < 1700)
                act_disarm();
        } else {
            if (!(d.channels[config_RC_CHAN_ARM] < 1700))
                act_arm();
        }
    } else {
        // TODO: hower
        ref = (llc_ThrustVec) {
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
        case ctrl_Idle: return (llc_ThrustVec) { 0 };

        case ctrl_RC: return _guide_rcMode();

        case ctrl_SoftLanding: err_todo(); break;
        case ctrl_GPS_Waypoint: err_todo(); break;
    }
}
