#include "err/err.h"

void err_fatal(const char* descr) {
    // TODO: err_fatal
    // if (sys_Initalized()) {
    //     sys_Abort();
    // } else {
    //     sys_Abort();
    // }
    tel_writeString(0, descr);
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
