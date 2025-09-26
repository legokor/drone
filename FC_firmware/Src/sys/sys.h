#ifndef SYS_H
#define SYS_H

#include "rc/rc.h"
#include "uart/uart.h"

extern uart_Uart sys_uartInstance;
extern rc_Rc sys_rcInstance;

void sys_entry(void);

typedef void (*sys_AbortFn)(void*);
[[noreturn]]
void sys_abort(sys_AbortFn fn, void* arg);

bool sys_initalized(void);

#endif // SYS_H
