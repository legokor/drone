#include "ctrl/ctrl.h"
#include "log/log.h"

static ctrl_Mode _ctrl_CurrentMode;

void ctrl_Init(void) {
    log_Debug("Initializing ctrl...");
}

void ctrl_SetMode(ctrl_Mode mode) {
    _ctrl_CurrentMode = mode;
}

ctrl_Mode ctrl_GetMode(void) {
    return _ctrl_CurrentMode;
}
