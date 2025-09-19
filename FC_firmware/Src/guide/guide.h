#ifndef GUIDE_H
#define GUIDE_H

#include "ctrl/ctrl.h"
#include "llc/llc.h"

void guide_init(void);

llc_ThrustVec guide_get_ref(ctrl_Mode mode);

#endif // GUIDE_H
