__asm__(".code16gcc");

#include "loader.h"

_boot_info_t boot_info;

static void show_msg (const char * msg){
    char c;

    while ((c = *msg ++) != '\0'){
        __asm__ __volatile__(
            "mov $0xe, %%ah\n\t"
            "mov %[ch], %%al\n\t"
            "int $0x10"::[ch]"r"(c)
        );
        
    }
}

static void detect_memory (void) {

    uint32_t contID =0;
    SMAP_entry_t smap_entry;
    uint32_t signature,bytes;

    show_msg("\rtry to detect memory:");

    boot_info.ram_region_count = 0;

    for (int i = 0; i < BOOT_RAM_REGION_MAX; i++){
        SMAP_entry_t * entry = &smap_entry;

        __asm__ __volatile__("int $0x15"
            : "=a"(signature),"=c"(bytes), "=b"(contID)
            : "a"(0xE820), "b"(contID), "c"(24), "d"(0x534D4150), "D"(entry));
        if (signature != 0x534D4150) {
            show_msg("failed\r\n");
            return;
        }
        if (bytes > 20 && (entry->ACPI & 0x0001) == 0) {
            continue;
        }
        if (entry->Type == 1) {
            boot_info.ram_region_cfg[boot_info.ram_region_count].start = entry->BaseL;
            boot_info.ram_region_cfg[boot_info.ram_region_count].size = entry->LengthL;
            boot_info.ram_region_count++;
        }
        if (contID == 0) {
            break;
        }
    }
    show_msg("ok.\r\n");

}

uint16_t gdt_table[][4] = {
    {0, 0, 0, 0},
    {0xFFFF, 0X0000, 0X9a00, 0X00cf},
    {0xFFFF, 0X0000, 0X9200, 0X00cf},
}; 

static void enter_protect_mode (void) {

    // 实模式切换到保护模式需要遵循一定流程 
    // ① 禁用中断 ② 打开A20地址线 ③ 加载GDT表 ④ 设置CR0保护模式使能位 ⑤ 远跳转清空流水线，取消16位指令
    //1
    cli();
    //2
    uint8_t v = inb(0x92);
    outb(0x92, v | 0x2);
    //3
    lgdt((uint32_t)gdt_table, sizeof(gdt_table)); 
    //4
    uint32_t cr0 = read_cr0();
    write_cr0(cr0 | (1 << 0));
    //5
    far_jump(8, (uint32_t)protect_mode_entry);

}

void loader_entry (void) {

    show_msg("\r==loader is loading==\n");

    detect_memory();

    enter_protect_mode();

    for (;;){}
} 