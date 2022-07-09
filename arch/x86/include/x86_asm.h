#ifndef __TINY_KERNEL_X86_ASM_H__
#define __TINY_KERNEL_X86_ASM_H__

#include "type.h"

static inline u8 inb(u16 port)
{
    u8 data;
    asm volatile("in %1, %0" : "=a"(data) : "d"(port));
    return data;
}

static inline void outb(u16 port, u8 data)
{
    asm volatile("out %0, %1" : : "a"(data), "d"(port));
}

static inline void outw(u16 port, u16 data)
{
    asm volatile("out %0, %1" : : "a"(data), "d"(port));
}

static inline void insl(u16 port, void *addr, u32 cnt)
{
    asm volatile("cld; rep insl"
                 : "=D"(addr), "=c"(cnt)
                 : "d"(port), "0"(addr), "1"(cnt)
                 : "memory", "cc");
}

static inline void outsl(u16 port, void *addr, u32 cnt)
{
    asm volatile("cld; rep outsl"
                 : "=S"(addr), "=c"(cnt)
                 : "d"(port), "0"(addr), "1"(cnt)
                 : "cc");
}

static inline void stosb(void *addr, u8 data, u32 cnt)
{
    asm volatile("cld; rep stosb"
                 : "=D"(addr), "=c"(cnt)
                 : "a"(data), "0"(addr), "1"(cnt)
                 : "memory", "cc");
}

static inline void stosl(void *addr, u32 data, u32 cnt)
{
    asm volatile("cld; rep stosl"
                 : "=D"(addr), "=c"(cnt)
                 : "a"(data), "0"(addr), "1"(cnt)
                 : "memory", "cc");
}

static inline void lcr3(u32 addr)
{
    asm volatile("movl %0, %%cr3" : : "r"(addr));
}

static inline void int0x22(void)
{
    asm volatile("int $0x22");
}

static inline void lgdt(uaddr pd)
{
    asm volatile("lgdt (%0)" : : "r"(pd));
}

static inline void lidt(uaddr pd)
{
    asm volatile("lidt (%0)" : : "r"(pd));
}

static inline void reload_seg(u32 code, u32 data)
{
    /*
    copy from linux-1.0
    iret equal to:
        popl eip
        popl cs
        popl eflag
        popl esp
        popl ss
    */
    asm volatile("movl %%esp, %%eax\n\t"
                 "pushl %0\n\t"
                 "pushl %%eax\n\t"
                 "pushfl\n\t"
                 "pushl %1\n\t"
                 "pushl $1f\n\t"
                 "iret\n\t"
                 "1:\n\t"
                 "movl %0, %%eax\n\t"
                 "mov %%ax, %%ds\n\t"
                 "mov %%ax, %%es\n\t"
                 "mov %%ax, %%fs\n\t"
                 "mov %%ax, %%gs" : : "i"(data), "i"(code):"ax");
}

static inline void cli(void)
{
    asm volatile("cli");
}

static inline void sti(void)
{
    asm volatile("sti");
}

static inline u32 save_flags(void)
{
    u32 flags;
    asm volatile("pushfl ; popl %0" : "=r"(flags) : : "memory");
    return flags;
}

static inline void restore_flags(u32 flag)
{
    asm volatile("pushl %0 ; popfl" : : "r"(flag) : "memory");
}

static inline void ltr(u16 sel)
{
    asm volatile("ltr %0" : : "r"(sel));
}

#endif
