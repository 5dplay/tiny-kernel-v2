#ifndef __TINY_KERNEL_X86_TRAP_H__
#define __TINY_KERNEL_X86_TRAP_H__

#include "type.h"
#include "x86_seg.h"

// x86 trap and interrupt constants.
// Processor-defined:
#define T_DIVIDE         0      // divide error
#define T_DEBUG          1      // debug exception
#define T_NMI            2      // non-maskable interrupt
#define T_BRKPT          3      // breakpoint
#define T_OFLOW          4      // overflow
#define T_BOUND          5      // bounds check
#define T_ILLOP          6      // illegal opcode
#define T_DEVICE         7      // device not available
#define T_DBLFLT         8      // double fault
// #define T_COPROC      9      // reserved (not used since 486)
#define T_TSS           10      // invalid task switch segment
#define T_SEGNP         11      // segment not present
#define T_STACK         12      // stack exception
#define T_GPFLT         13      // general protection fault
#define T_PGFLT         14      // page fault
// #define T_RES        15      // reserved
#define T_FPERR         16      // floating point error
#define T_ALIGN         17      // aligment check
#define T_MCHK          18      // machine check
#define T_SIMDERR       19      // SIMD floating point error

// These are arbitrarily chosen, but with care not to overlap
// processor defined exceptions or interrupt vectors.
#define T_SYSCALL       64      // system call, int $0x40
#define T_DEFAULT      500      // catchall

#define T_IRQ0          32      // IRQ 0 corresponds to int T_IRQ

#define IRQ_TIMER        0
#define IRQ_KBD          1
#define IRQ_COM1         4
#define IRQ_IDE         14
#define IRQ_IDE1        15
#define IRQ_ERROR       19

struct idt_gate_desc {
    u16 offset1;
    u16 selector;
    u16 flags;
    u16 offset2;
} __attribute__((packed));

struct idtr {
    u16 size;
    u32 offset;
} __attribute__((packed));

#define IDT_ENTRY_BUILTIN 32
#define IDT_ENTRY_MAX 256

static void inline set_idt_gate_desc(struct idt_gate_desc *desc,
                                     u16 type, u16 dpl, uaddr handler)
{
    desc->offset1 = handler & 0xFFFF;
    desc->offset2 = (handler >> 16) & 0xFFFF;
    desc->selector = SEG_KCODE << 3;
    desc->flags = 0x8000 | (dpl << 13) | (type << 8);
}

#define set_intr_gate(addr, handler) \
        set_idt_gate_desc(addr, 14, 0, handler)

#define set_trap_gate(addr, handler) \
        set_idt_gate_desc(addr, 15, 0, handler)

#define set_system_gate(addr, handler) \
        set_idt_gate_desc(addr, 15, 3, handler)

//NOTE: 取结构体的操作是 addr + offsetof(field), 而esp 压栈是 esp -= 4 的操作, 所以这里先入栈的在下面, 后入栈的在上面
//FIXME: 为何需要这些寄存器的值?
struct trap_frame {
    //pushal => eax, ecx, edx, ebx, ori_esp, ebp, esi, edi
    u32 edi;
    u32 esi;
    u32 ebp;
    u32 ori_esp;
    u32 ebx;
    u32 edx;
    u32 ecx;
    u32 eax;

    //pushl ds, es, fs, gs, 这几个仅用低16位
    u32 gs;
    u32 fs;
    u32 es;
    u32 ds;

    u32 trap_no;   //中断号, 由vector.S传入

    u32 error_code;//x86定义部分exception拥有error code, 为了统一,所有中断和异常都拥有error code,只不过默认为0
    u32 eip;
    u32 cs; //仅用低16位
    u32 eflags;

    //如果涉及到ring级别切换,还有esp和ss, FIXME: 原因是什么?
    u32 esp;
    u32 ss; //仅用低16位
};

typedef void (*trap_handler)(struct trap_frame *);

#endif
