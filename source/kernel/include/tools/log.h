#ifndef LOG_H
#define LOG_H

#include "comm/types.h"
#include "comm/cpu_instr.h"
#include "os_cfg.h"
#include "cpu/irq.h"

void log_init (void);
void log_printf(const char * fmt, ...);

#define COM1_PORT 0X3F8

#endif