#include "sys/sys.h"
#include "log/log.h"

sys_System sys_Instance;

void init_hardware() {
    log_Debug("Initalizing hardware...");

//    lora_Init(&sys_Instance.lora);
//    sd_Init(&sys_Instance.sd);
//    rc_Init(&sys_Instance.rc);
//    gps_Init(&sys_Instance.gps);
//    mag_Init(&sys_Instance.mag);
//    bar_Init(&sys_Instance.bar);
//    esc_Init(&sys_Instance.esc);
//    acc_Init(&sys_Instance.acc);
    imu_Init(&sys_Instance.imu);
    uart_Init(&sys_Instance.uart);
}

void init_modules() {
    log_Debug("Initalizing software modules...");

//    tel_Init();
//    act_Init();
//    llc_Init();
//    guide_Init();
//    ctrl_Init();
//    dsp_Init();
}

void init() {
    log_Debug("Initalizing...");

    init_hardware();
    init_modules();
}

void sys_Entry(void) {
    init();
}
