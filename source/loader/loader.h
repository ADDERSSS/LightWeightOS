#ifndef LOADER_H
#define LOADER_H

#include "comm/boot_info.h"
#include "comm/types.h"
#include "comm/cpu_instr.h"

void protect_mode_entry (void);
typedef struct SMAP_entry {

    unit32_t BaseL;
    unit32_t BaseH;
    unit32_t LengthL;
    unit32_t LengthH;
    unit32_t Type;
    unit32_t ACPI;

}__attribute__((packed)) SMAP_entry_t;


#endif