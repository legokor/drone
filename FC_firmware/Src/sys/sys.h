#ifndef SYS_H
#define SYS_H

#include "uart/uart.h"

extern uart_Uart sys_uartInstance;

void sys_entry(void);

#endif // SYS_H
