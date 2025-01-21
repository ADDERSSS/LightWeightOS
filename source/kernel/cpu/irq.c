#include "cpu/irq.h"
#include "cpu/cpu.h"

#define IDT_TABLE_NR 128

static gate_desc_t idt_table[IDT_TABLE_NR];

static void do_default_handler (exception_frame_t * frame, const char * message) {
    for (;;) {
        hlt();
    }
}

void do_handler_unknown (exception_frame_t * frame) {
    do_default_handler(frame, "unknown exception");
}

void do_handler_divider(exception_frame_t *frame) {
    do_default_handler(frame, "Divider exception");
}

void do_handler_Debug(exception_frame_t *frame) {
    do_default_handler(frame, "Debug exception");
}

void do_handler_NMI(exception_frame_t *frame) {
    do_default_handler(frame, "NMI exception");
}

void do_handler_breakpoint(exception_frame_t *frame) {
    do_default_handler(frame, "Breakpoint exception");
}

void do_handler_overflow(exception_frame_t *frame) {
    do_default_handler(frame, "Overflow exception");
}

void do_handler_bound_range(exception_frame_t *frame) {
    do_default_handler(frame, "Bound range exception");
}

void do_handler_invalid_opcode(exception_frame_t *frame) {
    do_default_handler(frame, "Invalid opcode exception");
}

void do_handler_device_unavailable(exception_frame_t *frame) {
    do_default_handler(frame, "Device unavailable exception");
}

void do_handler_double_fault(exception_frame_t *frame) {
    do_default_handler(frame, "Double fault exception");
}

void do_handler_invalid_tss(exception_frame_t *frame) {
    do_default_handler(frame, "Invalid TSS exception");
}

void do_handler_segment_not_present(exception_frame_t *frame) {
    do_default_handler(frame, "Segment not present exception");
}

void do_handler_stack_segment_fault(exception_frame_t *frame) {
    do_default_handler(frame, "Stack segment fault exception");
}

void do_handler_general_protection(exception_frame_t *frame) {
    do_default_handler(frame, "General protection exception");
}

void do_handler_page_fault(exception_frame_t *frame) {
    do_default_handler(frame, "Page fault exception");
}

void do_handler_fpu_error(exception_frame_t *frame) {
    do_default_handler(frame, "FPU error exception");
}

void do_handler_alignment_check(exception_frame_t *frame) {
    do_default_handler(frame, "Alignment check exception");
}

void do_handler_machine_check(exception_frame_t *frame) {
    do_default_handler(frame, "Machine check exception");
}

void do_handler_simd_exception(exception_frame_t *frame) {
    do_default_handler(frame, "SIMD exception");
}

void do_handler_virtual_exception(exception_frame_t *frame) {
    do_default_handler(frame, "Virtual exception");
}

void do_handler_control_exception(exception_frame_t *frame) {
    do_default_handler(frame, "Control exception");
}


int irq_install (int irq_num, irq_handler_t handler) {
    if (irq_num >= IDT_TABLE_NR) {
        return -1;
    }

    gate_desc_set(idt_table + irq_num, KERNEL_SELECTOR_CS, (uint32_t)handler,
    GATE_P_PRESENT | GATE_DPL0 | GATE_TYPE_INT);

    return 0;

}

static void init_pic (void) {
    outb(PIC0_ICW1, PIC_ICW1_ALWAYS_1 | PIC_ICW1_ICW4);
    outb(PIC0_ICW2, IRQ_PIC_START);
    outb(PIC0_ICW3, 1 << 2);
    outb(PIC0_ICW4, PIC_ICW4_8086);
    
    outb(PIC1_ICW1, PIC_ICW1_ALWAYS_1 | PIC_ICW1_ICW4);
    outb(PIC1_ICW2, IRQ_PIC_START + 8);
    outb(PIC1_ICW3, 2);
    outb(PIC1_ICW4, PIC_ICW4_8086);

    outb(PIC0_IMR, 0XFF & ~(1 << 2));
    outb(PIC1_IMR, 0XFF);
}

void irq_init (void) {
    for (int i = 0; i < IDT_TABLE_NR; i ++) {
        gate_desc_set(idt_table + i, KERNEL_SELECTOR_CS, (uint32_t)exception_handler_unknown,
        GATE_P_PRESENT | GATE_DPL0 | GATE_TYPE_INT);
    }

    irq_install(IRQ0_DE, (irq_handler_t)exception_handler_divider);
    irq_install(IRQ1_DB, (irq_handler_t)exception_handler_Debug);
    irq_install(IRQ2_NMI, (irq_handler_t)exception_handler_NMI);
    irq_install(IRQ3_BP, (irq_handler_t)exception_handler_breakpoint);
    irq_install(IRQ4_OF, (irq_handler_t)exception_handler_overflow);
    irq_install(IRQ5_BR, (irq_handler_t)exception_handler_bound_range);
    irq_install(IRQ6_UD, (irq_handler_t)exception_handler_invalid_opcode);
    irq_install(IRQ7_NM, (irq_handler_t)exception_handler_device_unavailable);
    irq_install(IRQ8_DF, (irq_handler_t)exception_handler_double_fault);
    irq_install(IRQ10_TS, (irq_handler_t)exception_handler_invalid_tss);
    irq_install(IRQ11_NP, (irq_handler_t)exception_handler_segment_not_present);
    irq_install(IRQ12_SS, (irq_handler_t)exception_handler_stack_segment_fault);
    irq_install(IRQ13_GP, (irq_handler_t)exception_handler_general_protection);
    irq_install(IRQ14_PF, (irq_handler_t)exception_handler_page_fault);
    irq_install(IRQ16_MF, (irq_handler_t)exception_handler_fpu_error);
    irq_install(IRQ17_AC, (irq_handler_t)exception_handler_alignment_check);
    irq_install(IRQ18_MC, (irq_handler_t)exception_handler_machine_check);
    irq_install(IRQ19_XM, (irq_handler_t)exception_handler_simd_exception);
    irq_install(IRQ20_VE, (irq_handler_t)exception_handler_virtual_exception);
    irq_install(IRQ21_CP, (irq_handler_t)exception_handler_control_exception);

    lidt((uint32_t)idt_table, sizeof(idt_table));

    init_pic();

}