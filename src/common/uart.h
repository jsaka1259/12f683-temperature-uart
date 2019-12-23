#ifndef _UART_H_
#define _UART_H_

#include <xc.h>
#include <stdint.h>

#include "xtal.h"

// Detect Delay Time
#define DTIME 10

// Baudrate
#define BAUDRATE 9600
#define BAUD     (uint8_t)(((_XTAL_FREQ / BAUDRATE) / 16) - 1)

#define TXPIN GP1
#define RXPIN GP0

extern void uart_init(void);
extern void uart_putc(const char data);
extern void uart_puts(const char *buf);
extern char uart_getc(void);

#endif
