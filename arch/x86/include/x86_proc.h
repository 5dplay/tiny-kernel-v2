#ifndef __TINY_KERNEL_X86_PROC_H__
#define __TINY_KERNEL_X86_PROC_H__

#include "x86_trap.h"

struct x86_context {
    u32 edi;
    u32 esi;
    u32 ebx;
    u32 ebp;
    u32 eip;
};
struct x86_proc {
    u8 *kstack;
    struct trap_frame *tf;
    struct x86_context *ctx;
};

#endif
