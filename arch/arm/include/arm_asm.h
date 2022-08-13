#ifndef __TINY_KERNEL_ARM_ASM_H__
#define __TINY_KERNEL_ARM_ASM_H__

#include "type.h"

static inline void mcr_ttbr0(u32 addr)
{
    asm volatile("mcr p15, 0, %0, c2, c0, 0" : : "r"(addr));
    asm volatile("mcr p15, 0, %0, c2, c0, 1" : : "r"(addr));
}
#endif
