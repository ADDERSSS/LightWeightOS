#ifndef FS_H
#define FS_H

#include "comm/cpu_instr.h"
#include "comm/boot_info.h"
#include "comm/elf.h"
#include "comm/types.h"

int sys_open (const char * name, int flags, ...);
int sys_read (int file, char * ptr, int len);
int sys_write (int file, char * ptr, int len);
int sys_lseek(int file, int ptr, int dir);
int sys_close(int file);

#endif