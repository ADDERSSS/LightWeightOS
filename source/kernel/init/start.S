    .text
    .extern kernel_init
    .global _start
    # void start (_boot_info_t * boot_info)
_start:
    push %ebp
    mov %esp, %ebp
    mov 0x8(%ebp), %eax

    # mov 4(%esp), %eax
    
    push %eax 
    call kernel_init
    jmp .
