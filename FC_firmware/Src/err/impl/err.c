#include "err/err.h"
#include "sys/sys.h"
#include "tel/tel.h"
}

void err_ignorable(const char* descr) {
    // TODO: err_ignorable
    // if (sys_Initalized()) {
    //     sys_Abort();
    // } else {
    //     sys_Abort();
    // }
    tel_writeString(0, descr);
}
