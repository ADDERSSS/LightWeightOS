#include "tools/log.h"
#include "core/task.h"

int first_task_main (void) {
    for(;;) {
        log_printf("first task.");
        sys_sleep(1000);
    }
    return 0;
}