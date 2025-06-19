#include "err/err.h"
#include "sys/sys.h"
#include "tel/tel.h"

typedef struct [[nodiscard]] _err_abort_fn_params {
    const char* descr;
} _err_abort_fn_params;

void _err_abort_fn(void* args) {
    _err_abort_fn_params* params = (_err_abort_fn_params*) args;
    tel_writeString(0, params->descr);
}

void _err_abort(const char* descr) {
    _err_abort_fn_params params = { descr };
    sys_abort(_err_abort_fn, &params);
}

void err_handle_fatal(const char* descr) {
    _err_abort(descr);
}

void err_handle_ignorable(const char* descr) {
    if (sys_initalized()) {
        tel_writeString(0, descr);
    } else {
        _err_abort(descr);
    }
}
