#ifndef SYS_H
#define SYS_H

#include "acc/acc.h"
#include "bar/bar.h"
#include "esc/esc.h"
#include "gps/gps.h"
#include "imu/imu.h"
#include "lora/lora.h"
#include "mag/mag.h"
#include "rc/rc.h"

#include "act/act.h"
#include "ctrl/ctrl.h"
#include "dsp/dsp.h"
#include "guide/guide.h"
#include "llc/llc.h"
#include "tel/tel.h"

typedef struct sys_System {
    gps_Gps gps;
    mag_Mag mag;
    acc_Acc acc;
    imu_Imu imu;
    bar_Bar bar;
    lora_Lora lora;
    esc_Esc esc;
    rc_Rc rc;

    ctrl_Ctrl ctrl;
    dsp_Dsp dsp;
    act_Act act;
    llc_Llc llc;
    tel_Tel tel;
    guide_Guide guide;
} sys_System;

sys_System sys_Instance;

void sys_Entry(void);

#endif // SYS_H
