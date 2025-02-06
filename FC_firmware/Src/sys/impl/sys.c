#include "sys.h"
#include "log.h"

void init_hardware(sys_System* sys) {
    log_Debug("Initalizing hardware...");

    lora_Init();
    sd_Init();
    imu_Init();
    rc_Init();
    gps_Init();
    mag_Init();
    bar_Init();
    esc_Init();
    acc_Init();
}

void init_modules(sys_System* sys) {
    log_Debug("Initalizing software modules...");

    tel_Init();
    act_Init();
    llc_Init();
    guide_Init();
    ctrl_Init();
    dsp_Init();
}

void init(sys_System* sys) {
    log_Debug("Initalizing...");

    init_hardware(sys);
    init_modules(sys);
}

void sys_Entry(void) {
    init(&sys_Instance);
}
