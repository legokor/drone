#ifndef MGMT_H
#define MGMT_H

#include "acc.h"
#include "bar.h"
#include "esc.h"
#include "gps.h"
#include "imu.h"
#include "lora.h"
#include "mag.h"
#include "rc.h"

#include "act.h"
#include "ctrl.h"
#include "dsp.h"
#include "guide.h"
#include "llc.h"
#include "tel.h"

typedef struct mgmt_System {
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
} mgmt_System;

void mgmt_Entry(void);

#endif // MGMT_H
