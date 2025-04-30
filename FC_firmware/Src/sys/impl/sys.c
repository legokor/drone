#include "sys/sys.h"
#include "log/log.h"

#include "act/act.h"
#include "ctrl/ctrl.h"
#include "dsp/dsp.h"
#include "guide/guide.h"
#include "llc/llc.h"
#include "tel/tel.h"

static gps_Gps _sys_Gps;
static mag_Mag _sys_Mag;
static acc_Acc _sys_Acc;
static imu_Imu _sys_Imu;
static bar_Bar _sys_Bar;
static lora_Lora _sys_Lora;
static esc_Esc _sys_Esc;
static rc_Rc _sys_Rc;
static sd_Sd _sys_Sd;

void init_hardware(void) {
    log_Debug("Initalizing hardware...");

    lora_Init(&_sys_Lora);
    sd_Init(&_sys_Sd);
    imu_Init(&_sys_Imu);
    rc_Init(&_sys_Rc);
    gps_Init(&_sys_Gps);
    mag_Init(&_sys_Mag);
    bar_Init(&_sys_Bar);
    esc_Init(&_sys_Esc);
    acc_Init(&_sys_Acc);
}

void init_modules(void) {
    log_Debug("Initalizing software modules...");

    tel_Init();
    act_Init();
    llc_Init();
    guide_Init();
    ctrl_Init();
    dsp_Init();
}

void init(void) {
    log_Debug("Initalizing...");

    init_hardware();
    init_modules();
}

void sys_Entry(void) {
    init();
}
