#ifndef ELF_H
#define ELF_H

#include "types.h"

#define EI_NIDENT 16
typedef uint32_t Elf32_Addr;
typedef uint32_t Elf32_off;
typedef uint16_t Elf32_Half;
typedef uint32_t Elf32_Word;

#define PT_LOAD 1 

#pragma pack(1)

typedef struct {
    unsigned char e_ident[EI_NIDENT];
    Elf32_Half e_type;
    Elf32_Half emachine;
    Elf32_Word e_version;
    Elf32_Addr e_entry;
    Elf32_off e_phoff;
    Elf32_off e_shoff;
    Elf32_Word e_flags;
    Elf32_Half e_ehsize;
    Elf32_Half e_phentsize;
    Elf32_Half e_phnum;
    Elf32_Half e_shentsize;
    Elf32_Half e_shnum;
    Elf32_Half e_shstrndx;
}ELF32_Ehdr;

typedef struct {
    Elf32_Word p_type;
    Elf32_off p_offset;
    Elf32_Addr p_vaddr;
    Elf32_Addr p_paddr;
    Elf32_Word p_filesz;
    Elf32_Word p_memsz;
    Elf32_Word p_flags;
    Elf32_Word p_align;
}Elf32_Phdr;

#pragma pack()

#endif 