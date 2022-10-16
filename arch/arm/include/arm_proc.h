#ifndef __TINY_KERNEL_ARM_PROC_H__
#define __TINY_KERNEL_ARM_PROC_H__

#include "arm_trap.h"

struct arm_context {
    u32 r4;
    u32 r5;
    u32 r6;
    u32 r7;
    u32 r8;
    u32 r9;
    u32 r10;
    u32 r11;
    u32 r12;
    u32 lr;
};

struct arm_proc {
    u8 *kstack;
    struct trap_frame *tf;
    struct arm_context *ctx;
};

#endif