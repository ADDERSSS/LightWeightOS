#ifndef SYSCALL_H
#define SYSCALL_H

#include "comm/types.h"

#define SYSCALL_PARAM_COUNT 5
#define SYS_sleep 0
#define SYS_getpid 1
#define SYS_fork 2
#define SYS_execve 3
#define SYS_yield 4
#define SYS_exit 5
#define SYS_wait 6

#define SYS_open 50
#define SYS_read 51
#define SYS_write 52
#define SYS_close 53
#define SYS_lseek 54
#define SYS_isatty 55
#define SYS_sbrk 56
#define SYS_fstat 57
#define SYS_dup 58
#define SYS_ioctl 59

#define SYS_opendir 60
#define SYS_readdir 61
#define SYS_closedir 62
#define SYS_unlink 63

#define SYS_printmsg 100

typedef struct _syscall_frame_t {
    int eflags;
    int gs, fs, es, ds;
    uint32_t edi, esi, ebp, dummy, ebx, edx, ecx, eax;
    int eip, cs;
    int func_id, arg0, arg1, arg2, arg3;
    int esp, ss;
}syscall_frame_t;

typedef int (* syscall_handler_t)(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3);

void exception_handler_syscall (void);

#endif 