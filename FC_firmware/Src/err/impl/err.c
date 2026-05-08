#include "err/err.h"
#include "log/log.h"
#include "sys/sys.h"

typedef struct [[nodiscard]] _err_abort_fn_params {
    const char* descr;
} _err_abort_fn_params;

static void _err_abort_fn(void* args) {
    _err_abort_fn_params* params = (_err_abort_fn_params*) args;
    log_error(params->descr);
}

[[noreturn]]
static void _err_abort(const char* descr) {
    _err_abort_fn_params params = { descr };
    sys_abort(_err_abort_fn, &params);
}

void err_handle_fatal(const char* descr) {
    _err_abort(descr);
}

void err_handle_ignorable(const char* descr) {
    if (sys_initalized()) {
        log_error(descr);
    } else {
        _err_abort(descr);
    }
}
