#include "init.h"
#include "comm/boot_info.h"
#include "cpu/cpu.h"
#include "cpu/irq.h"
#include "dev/time.h"
#include "tools/log.h"

void kernel_init (_boot_info_t * boot_info) {
    cpu_init();
    log_init();
    irq_init();
    time_init();

    
}

void init_main (void) {
    log_printf("Kernel is running ...");
    log_printf("Version: %s %s", OS_VERSION, "LightWeightOS");
    // int a = 3 / 0;
    // irq_enable_global();
    for (;;) {}
}