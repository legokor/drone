#ifndef SYS_H
#define SYS_H

#include "uart/uart.h"

extern uart_Uart sys_uartInstance;

void sys_entry(void);

typedef void (*sys_AbortFn)(void*);
void sys_abort(sys_AbortFn fn, void*arg);

bool sys_initalized(void);

#endif // SYS_H
