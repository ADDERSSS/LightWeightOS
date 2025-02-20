#include "tools/log.h"
#include "core/task.h"
#include "applib/lib_syscall.h"

int first_task_main (void) {
    int count = 3;
    int pid = getpid();
    print_msg("HELLO first id -%d-", pid);

    int fpid = fork();

    if (fpid < 0) {
        print_msg("failed-%d-", 0);
    } else if (fpid == 0) {
        print_msg("child-%d-", count);
    } else {
        print_msg("child task id -%d-", fpid);
        print_msg("parent-%d-", count);
    }

    for(;;) {
        print_msg("task id=%d", pid);
        msleep(1000);
    }
    return 0;
}