#include "sys.h"
#include "log.h"

void init_hardware(sys_System* sys) {
    log_Debug("Initalizing hardware...");

    lora_Init(&sys->lora);
    imu_Init(&sys->imu);
    rc_Init(&sys->rc);
    gps_Init(&sys->gps);
    mag_Init(&sys->mag);
    bar_Init(&sys->bar);
    esc_Init(&sys->esc);
    acc_Init(&sys->acc);
}

void init_modules(sys_System* sys) {
    log_Debug("Initalizing software modules...");

    tel_Init(&sys->tel);
    act_Init(&sys->act);
    llc_Init(&sys->llc);
    guide_Init(&sys->guide);
    ctrl_Init(&sys->ctrl);
    dsp_Init(&sys->dsp);
}

void init(sys_System* sys) {
    log_Debug("Initalizing...");

    init_hardware(sys);
    init_modules(sys);
}

void sys_Entry(void) {
    sys_System sys;

    init(&sys);
}
