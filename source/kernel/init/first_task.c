#include "tools/log.h"
#include "core/task.h"
#include "applib/lib_syscall.h"

int first_task_main (void) {
    int count = 3;
    int pid = getpid();
    print_msg("HELLO first id -%d-", pid);

    pid = fork();

    if (pid < 0) {
        print_msg("create failed-%d-", 0);
    } else if (pid == 0) {
        print_msg("child-%d-", count);

        char * argv[] = {"arg0", "arg1", "arg2", "arg3"};
        execve("/shell.elf", argv, (char **)0);
    } else {
        print_msg("child task id -%d-", pid);
        print_msg("parent-%d-", count);
    }

    for(;;) {
        print_msg("task id=%d", pid);
        msleep(1000);
    }
    return 0;
} 