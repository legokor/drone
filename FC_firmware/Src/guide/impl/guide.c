#include "guide/guide.h"
#include "err/err.h"
#include "llc/llc.h"
#include "log/log.h"
#include "rc/rc.h"
#include "sys/sys.h"

#include <stdint.h>

void guide_init(void) {
    log_debug("Initializing guide...");
}

static llc_ThrustVec _guide_rcMode() {
    rc_RxPackage d;
    err_tryIgnorable(rc_getData(&sys_rcInstance, &d), "Failed to get rc data");

#define _guide_c(c) ((d.channels[c]) / (float) UINT16_MAX)

    // TODO: correct channels
    llc_ThrustVec ref = (llc_ThrustVec) {
        .roll = _guide_c(0),
        .pitch = _guide_c(1),
        .yaw = _guide_c(2),
        .thrust = _guide_c(3),
    };

    llc_set_consts(_guide_c(4), _guide_c(5), _guide_c(6), //
                   _guide_c(7), _guide_c(8), _guide_c(9), //
                   _guide_c(10), _guide_c(11), _guide_c(12));

#undef _guide_c

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
