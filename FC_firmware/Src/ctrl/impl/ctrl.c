#include "ctrl/ctrl.h"
#include "act/act.h"
#include "log/log.h"

static ctrl_Mode _ctrl_CurrentMode;

void ctrl_init(void) {
    log_debug("Initializing ctrl...");
}

void ctrl_setMode(ctrl_Mode mode) {
    // switching to/from idle
    if (mode == ctrl_Idle)
        act_disarm();
    else if (ctrl_getMode() == ctrl_Idle)
        act_arm();

    _ctrl_CurrentMode = mode;
}

ctrl_Mode ctrl_getMode(void) {
    return _ctrl_CurrentMode;
}
