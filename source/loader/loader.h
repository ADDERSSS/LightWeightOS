#ifndef LOADER_H
#define LOADER_H

#include "comm/boot_info.h"
#include "comm/types.h"
#include "comm/cpu_instr.h"
#include "comm/elf.h"

void protect_mode_entry (void);
typedef struct SMAP_entry {

    uint32_t BaseL;
    uint32_t BaseH;
    uint32_t LengthL;
    uint32_t LengthH;
    uint32_t Type;
    uint32_t ACPI;

}__attribute__((packed)) SMAP_entry_t;

extern _boot_info_t boot_info;

#endif