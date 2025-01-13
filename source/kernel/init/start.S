    .text
    .extern kernel_init
    .global _start
    # void start (_boot_info_t * boot_info)
_start:

    push 4(%esp)

    call kernel_init

    jmp .
