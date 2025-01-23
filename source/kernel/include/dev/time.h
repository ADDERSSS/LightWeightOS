#ifndef TIME_H
#define TIME_H

#include "comm/types.h"
#include "comm/cpu_instr.h"
#include "os_cfg.h"
#include "cpu/irq.h"

#define PIT_OSC_FREQ 1193182
#define PIT_COMMAND_MODE_PORT 0X43
#define PIT_CHANNEL0_DATA_PORT 0X40

#define PIT_CHANNEL (0 << 6)
#define PIT_LOAD (3 << 4)
#define PIT_MODE3 (3 << 1)

void time_init (void);

void exception_handler_time (void);

#endif