#include "guide/guide.h"
#include "err/err.h"
#include "llc/llc.h"
#include "log/log.h"
#include "rc/rc.h"
#include "sys/sys.h"

#include "config.h"

void guide_init(void) {
    log_debug("Initializing guide...");
}

static llc_ThrustVec _guide_rcMode() {
    rc_RxPackage d;

    // TODO: fatal?
    bool gotRC = rc_getData(&sys_rcInstance, &d);
    err_tryIgnorable(gotRC, "Failed to get rc data");

    // FIXME
#define _guide_c(c) ((float) d.channels[c])

    llc_ThrustVec ref;
    if (gotRC) {

        // TODO: correct channels
        // TODO: disarm
        ref = (llc_ThrustVec) {
            .roll = _guide_c(CONFIG_RC_CHAN_ROLL),
            .pitch = _guide_c(CONFIG_RC_CHAN_PITCH),
            .yaw = _guide_c(CONFIG_RC_CHAN_YAW),
            .thrust = _guide_c(CONFIG_RC_CHAN_THRUST),
        };

        llc_set_consts(_guide_c(4), _guide_c(5), _guide_c(6), //
                       _guide_c(7), _guide_c(8), _guide_c(9), //
                       _guide_c(10), _guide_c(11), _guide_c(12));

#undef _guide_c
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
