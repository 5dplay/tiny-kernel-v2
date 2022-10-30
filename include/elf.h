#ifndef __TINY_KERNEL_ELF_H__
#define __TINY_KERNEL_ELF_H__

#include "type.h"

// Format of an ELF executable file
#define ELF_MAGIC 0x464C457FU  // "\x7FELF" in little endian

// File header
struct elfhdr {
    uint magic;  // must equal ELF_MAGIC
    u8 elf[12];
    u16 type;
    u16 machine;
    uint version;
    uint entry;
    uint phoff;
    uint shoff;
    uint flags;
    u16 ehsize;
    u16 phentsize;
    u16 phnum;
    u16 shentsize;
    u16 shnum;
    u16 shstrndx;
} __attribute__((packed));

// Program section header
struct proghdr {
    uint type;
    uint off;
    uint vaddr;
    uint paddr;
    uint filesz;
    uint memsz;
    uint flags;
    uint align;
} __attribute__((packed));

// Values for Proghdr type
#define ELF_PROG_LOAD           1

// Flag bits for Proghdr flags
#define ELF_PROG_FLAG_EXEC      1
#define ELF_PROG_FLAG_WRITE     2
#define ELF_PROG_FLAG_READ      4

#endif
