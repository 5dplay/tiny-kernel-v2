#ifndef __TINY_KERNEL_ARM_ASM_H__
#define __TINY_KERNEL_ARM_ASM_H__

#include "type.h"

static inline void mcr_ttbr0(u32 addr)
{
    asm volatile("mcr p15, 0, %0, c2, c0, 0" : : "r"(addr));
    asm volatile("mcr p15, 0, %0, c2, c0, 1" : : "r"(addr));
}

static inline void set_vec_base()
{
    asm volatile("mrc p15, 0, r0, c1, c0, 0 \t\n" \
                 "orr r0, r0, #1 << 13 \t\n" \
                 "mcr p15, 0, r0, c1, c0, 0 \t\n" \
                 "ldr r0, =0xffff0000 \t\n" \
                 "mcr p15, 0, r0, c12, c0, 0");
}

static inline void get_dfar(u32 *dfar)
{
    u32 addr;
    asm volatile("mrc p15, 0, %0, c6, c0, 0" : "=r"(addr) :);
    *dfar = addr;
}

static inline void get_cpsr(u32 *cpsr)
{
    u32 addr;
    asm volatile("mrs %0, cpsr" : "=r"(addr) :);
    *cpsr = addr;
}

static inline void set_cpsr(u32 cpsr)
{
    asm volatile("msr cpsr, %0" : : "r"(cpsr));
}
#endif
