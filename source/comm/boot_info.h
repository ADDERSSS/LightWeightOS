#ifndef BOOT_INFO_H
#define BOOT_INFO_H

#include "types.h"

#define BOOT_RAM_REGION_MAX 10

typedef struct _boot_info_t {
    struct {
        unit32_t start;
        unit32_t size;
    }ram_region_cfg[BOOT_RAM_REGION_MAX];
    
    int ram_region_count;
     
}_boot_info_t;

#endif