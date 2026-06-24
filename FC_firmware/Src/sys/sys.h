#ifndef SYS_H
#define SYS_H

#include "rc/rc.h"
#include "uart/uart.h"

typedef void (*sys_AbortFn)(void*);

extern uart_Uart sys_uartInstance;
extern rc_Rc sys_rcInstance;

[[noreturn]]
void sys_entry(void);

[[noreturn]]
void sys_abort(sys_AbortFn fn, void* arg);

bool sys_isInitalized(void);

#endif // SYS_H
