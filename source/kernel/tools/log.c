#include <stdarg.h>
#include "tools/log.h"
#include "tools/klib.h"
#include "ipc/mutex.h"
#include "dev/console.h"
#include "dev/dev.h"

#define LOG_USE_COM 0

static mutex_t log_mutex;

static int log_dev_id;

void log_init (void) {  
    mutex_init(&log_mutex);

    log_dev_id = dev_open(DEV_TTY, 0, (void *)0);

#if LOG_USE_COM
    outb(COM1_PORT + 1, 0X00);
    outb(COM1_PORT + 3, 0X80);
    outb(COM1_PORT + 0, 0X3);
    outb(COM1_PORT + 1, 0X00);
    outb(COM1_PORT + 3, 0X03);
    outb(COM1_PORT + 2, 0XC7);
    outb(COM1_PORT + 4, 0X0F);
#endif
}

void log_printf(const char * fmt, ...) {
    char str_buf[128];
    va_list args;

    kernel_memset(str_buf, '\0', sizeof(str_buf));
    va_start(args, fmt);
    kernel_vsprintf(str_buf, fmt, args);
    va_end(args);

    mutex_lock(&log_mutex);

#if LOG_USE_COM
    const char * p = str_buf;
    while (*p != '\0') {
        while ((inb(COM1_PORT + 5) & (1 << 6)) == 0);
        outb(COM1_PORT, *p++);
    }
    outb(COM1_PORT, '\r');
    outb(COM1_PORT, '\n');
#else
    //console_write(0, str_buf, kernel_strlen(str_buf));
    dev_write(log_dev_id, 0, "log:",4);
    dev_write(log_dev_id, 0, str_buf, kernel_strlen(str_buf));
    char c = '\n';
    //console_write(0, &c, 1);
    dev_write(log_dev_id, 0, &c, 1);
#endif

    mutex_unlock(&log_mutex);
}