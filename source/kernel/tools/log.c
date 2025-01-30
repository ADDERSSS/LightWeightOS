#include <stdarg.h>
#include "tools/log.h"
#include "tools/klib.h"

void log_init (void) {  
    outb(COM1_PORT + 1, 0X00);
    outb(COM1_PORT + 3, 0X80);
    outb(COM1_PORT + 0, 0X3);
    outb(COM1_PORT + 1, 0X00);
    outb(COM1_PORT + 3, 0X03);
    outb(COM1_PORT + 2, 0XC7);
    outb(COM1_PORT + 4, 0X0F);
}

void log_printf(const char * fmt, ...) {
    char str_buf[128];
    va_list args;

    kernel_memset(str_buf, '\0', sizeof(str_buf));
    va_start(args, fmt);
    kernel_vsprintf(str_buf, fmt, args);
    va_end(args);

    const char * p = str_buf;
    while (*p != '\0') {
        while ((inb(COM1_PORT + 5) & (1 << 6)) == 0);
        outb(COM1_PORT, *p++);
    }
    outb(COM1_PORT, '\r');
    outb(COM1_PORT, '\n');
}