/**
 * 系统引导部分，启动时由硬件加载运行，然后完成对二级引导程序loader的加载
 */
__asm__(".code16gcc");

#include "boot.h"

#define LOADER_START_ADDR 0x8000

/**
 * Boot的C入口函数
 * 只完成一项功能，即从磁盘找到loader文件然后加载到内容中，并跳转过去
 */
void boot_entry(void) {
    ((void (*)(void))LOADER_START_ADDR)();
} 

