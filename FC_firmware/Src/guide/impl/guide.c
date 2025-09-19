#include "guide/guide.h"
#include "err/err.h"
#include "log/log.h"

void guide_init(void) {
    log_debug("Initializing guide...");
}

llc_ThrustVec guide_get_ref(ctrl_Mode mode) {
    switch (mode) {
        case ctrl_Idle: return (llc_ThrustVec) { 0 };

        case ctrl_RC: break;

        case ctrl_SoftLanding: err_todo(); break;
        case ctrl_GPS_Waypoint: err_todo(); break;
    }
}
