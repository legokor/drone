#include "ctrl/ctrl.h"
#include "log/log.h"

static ctrl_Mode _ctrl_CurrentMode;

void ctrl_init(void) {
    log_debug("Initializing ctrl...");
}

void ctrl_setMode(ctrl_Mode mode) {
    _ctrl_CurrentMode = mode;
}

ctrl_Mode ctrl_getMode(void) {
    return _ctrl_CurrentMode;
}
