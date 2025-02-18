#include "init.h"

void kernel_init (_boot_info_t * boot_info) {
    ASSERT(boot_info->ram_region_count != 0);
    // ASSERT(3 < 2);
    cpu_init();
    log_init();
    memory_init(boot_info);
    irq_init();
    time_init();

    task_manager_init();
}

static uint32_t init_task_stack[1024];
static task_t init_task;
static sem_t sem;

void init_task_entry (void) {
    int count = 0;

    for (;;) {
        //sem_wait(&sem);
        log_printf("init task: %d", count++);
        //sys_sched_yield();
        //sys_sleep(100);
    }     
}

void list_test (void) {
    list_t list;
    list_node_t nodes[5];


    list_init(&list);
    log_printf("list: first=0x%x, last=0x%x, count=%d",
        list_first(&list), list_last(&list), list_count(&list));
// insert first to list
    for (int i = 0; i < 5; i++) {
        list_node_t * node = nodes + i;
        log_printf("insert first to list: %d, 0x%x", i, (uint32_t)node);
        list_insert_first(&list, node);
    }

    log_printf("list: first=0x%x, last=0x%x, count=%d",
        list_first(&list), list_last(&list), list_count(&list));
// insert last to list
    list_init(&list);
    for (int i = 0; i < 5; i++) {
        list_node_t * node = nodes + i;
        log_printf("insert last to list: %d, 0x%x", i, (uint32_t)node);
        list_insert_last(&list, node);
    }
    
    log_printf("list: first=0x%x, last=0x%x, count=%d",
        list_first(&list), list_last(&list), list_count(&list));
// remove first from list
    for (int i = 0; i < 5; i++) {
        list_node_t * node = list_remove_first(&list);
        log_printf("remove first from list: %d, 0x%x", i, (uint32_t)node);
    }

    log_printf("list: first=0x%x, last=0x%x, count=%d",
        list_first(&list), list_last(&list), list_count(&list));
// insert first to list     
    for (int i = 0; i < 5; i++) {
        list_node_t * node = nodes + i;
        log_printf("insert first to list: %d, 0x%x", i, (uint32_t)node);
        list_insert_first(&list, node);
    }

    log_printf("list: first=0x%x, last=0x%x, count=%d",
        list_first(&list), list_last(&list), list_count(&list));
// remove from list
    for (int i = 0; i < 5; i++) {
        list_node_t * node = nodes + i;
        list_node_t * remove_node = list_remove(&list, node);
        log_printf("remove from list: %d, 0x%x", i, (uint32_t)remove_node);
    }

    log_printf("list: first=0x%x, last=0x%x, count=%d",
        list_first(&list), list_last(&list), list_count(&list));

    struct type_t {
        int i;
        list_node_t node;
    }v = {0x123456};

    list_node_t * v_node = &v.node;
    struct type_t * p = list_node_parent(v_node, struct type_t, node);
    if(p->i == 0x123456) {
        log_printf("list_node_parent success");
    }


}

void init_main (void) {   

    // list_test();

    log_printf("Kernel is running ...");
    log_printf("Version: %s %s", OS_VERSION, "LightWeightOS");
    log_printf("%d %d %x %c", 123456, -123, 0x123, 'a');

    // int a = 3 / 0;

    task_init(&init_task, "init task", (uint32_t)init_task_entry, (uint32_t)&init_task_stack[1024]);
    task_first_init();

    sem_init(&sem, 0);

    irq_enable_global();

    int count = 0;
    for (;;) {
        log_printf("first task: %d", count++);
        //sem_notify(&sem);
        //sys_sched_yield();
        //sys_sleep(1000);
    } 
}