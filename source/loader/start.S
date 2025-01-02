
  	// 16位代码，务必加上
  	.code16
 	.text
	.global _start
	.extern loader_entry


_start:

	mov $0xe, %ah
	mov $'\n', %al
	int $0x10
	mov $'\r', %al
	int $0x10
	mov $'L', %al
	int $0x10
	mov $'O', %al
	int $0x10
	mov $'A', %al 
	int $0x10
	mov $'D', %al
	int $0x10
	mov $'\n', %al
	int $0x10

	jmp loader_entry

	.code32
	.text
	.global protect_mode_entry
	.extern load_kernel

protect_mode_entry:

    //设置寄存器，进入32位保护模式运行

    mov $16, %ax
	mov %ax, %ds
	mov %ax, %ss
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs

    //跳转到32位c环境

    jmp $8, $load_kernel

