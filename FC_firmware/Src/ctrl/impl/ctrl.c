#include "ctrl/ctrl.h"
#include "act/act.h"
#include "log/log.h"

static ctrl_Mode _ctrl_CurrentMode = ctrl_Idle;

void ctrl_init(void) {
    log_debug("Initializing ctrl...");
}

void ctrl_setMode(ctrl_Mode newMode) {
    // switching to idle
    if (newMode == ctrl_Idle)
        act_disarm();

    _ctrl_CurrentMode = newMode;
}

ctrl_Mode ctrl_getMode(void) {
    return _ctrl_CurrentMode;
}
