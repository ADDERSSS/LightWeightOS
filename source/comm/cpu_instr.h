#ifndef CPU_INSTR_H
#define CPU_INSTR_H

#include "types.h"

static inline unit8_t inb (unit16_t port) {
     unit8_t rv;
     
     // inb al, dx

     __asm__ __volatile__("inb %[p], %[v]" : [v]"=a"(rv) : [p]"d"(port));
     return rv;
}

static inline void outb (unit16_t port, unit8_t data) {
     // outb al, dx

     __asm__ __volatile__("outb %[v], %[p]" :: [p]"d"(port), [v]"a"(data));
}

static inline void cli (void) {
     __asm__ __volatile__("cli");
}

static inline void sti (void) {
     __asm__ __volatile__("sti");
}

static inline void lgdt (unit32_t start, unit32_t size) {

     struct {
          unit16_t limit;
          unit16_t start15_0;
          unit16_t start31_16;
     }gdt;

     gdt.start31_16 = start >> 16;
     gdt.start15_0 = start & 0xFFFF;
     gdt.limit = size - 1;

     __asm__ __volatile__("lgdt %[g]" :: [g]"m"(gdt));
}

static inline unit16_t read_cr0 (void) {
     unit32_t cr0;

     __asm__ __volatile__("mov %%cr0, %[v]" : [v]"=r"(cr0));
     return cr0;
}

static inline void write_cr0 (unit32_t v) {
     __asm__ __volatile__("mov %[v], %%cr0" :: [v]"r"(v));
}

static inline void far_jump (unit32_t selector, unit32_t offset) {
     unit32_t addr[] = {offset, selector};

     __asm__ __volatile__("ljmpl *(%[a])" :: [a]"r"(addr));
}

#endif