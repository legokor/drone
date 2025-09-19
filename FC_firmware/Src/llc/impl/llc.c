#include "llc/llc.h"
#include "log/log.h"

#include "arm_math.h"

// static arm_pid_instance_f32 _llc_pid;

void llc_init(void) {
    log_debug("Initializing llc...");

    // TODO: tune PID
    // _llc_pid.Kp = 1.0f / sys_ACT_FREQ;
    // _llc_pid.Ki = 0.0f / sys_ACT_FREQ;
    // _llc_pid.Kd = 0.0f / sys_ACT_FREQ;
    // arm_pid_init_f32(&_llc_pid, 1);
}

llc_ThrustVec llc_update(llc_ThrustVec ref) {
    // TODO: PIDs and stuff... use like this:
    // float err = ref - pv;
    // arm_pid_f32(&_llc_pid, err);

    return ref;
}
