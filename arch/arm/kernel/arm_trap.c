#include "memlayout.h"
#include "common.h"
#include "trap.h"
#include "mm.h"
#include "vm.h"
#include "arm_asm.h"
#include "arm_trap.h"

extern char arm_vector_start[], arm_vector_end[];
#define ARM_VECTOR_BASE 0xFFFF0000

extern void __trap_init();

TK_STATUS trap_init()
{
    kmap(ARM_VECTOR_BASE, virt_to_phy((uaddr)arm_vector_start), arm_vector_end - arm_vector_start, VM_PERM_USER);

    set_vec_base();

    return TK_STATUS_SUCCESS;
}

static void show_tf(struct trap_frame *tf)
{
    printk("pc: [<0x%08x>] lr: [<0x%08x>] cpsr: 0x%08x\n"
           "sp: 0x%08x ip: 0x%08x fp: 0x%08x\n",
           tf->pc, tf->lr, tf->cpsr, tf->sp, tf->ip, tf->fp);

    printk("r10: 0x%08x r9: 0x%08x r8: 0x%08x\n",
           tf->r10, tf->r9, tf->r8);
    printk("r7: 0x%08x r6: 0x%08x r5: 0x%08x r4: 0x%08x\n",
           tf->r7, tf->r6, tf->r5, tf->r4);
    printk("r3: 0x%08x r2: 0x%08x r1: 0x%08x r0: 0x%08x\n",
           tf->r3, tf->r2, tf->r1, tf->r0);
}

#define DECLARE_TRAP_HANDLER(func_name)                             \
void do_##func_name(struct trap_frame *tf)                          \
{                                                                   \
    panic(#func_name" trigger\n");                                  \
}

DECLARE_TRAP_HANDLER(trap_reset)
DECLARE_TRAP_HANDLER(undef_instruction)
DECLARE_TRAP_HANDLER(software_interrupt)
DECLARE_TRAP_HANDLER(prefetch_abort)
/* DECLARE_TRAP_HANDLER(data_abort) */
DECLARE_TRAP_HANDLER(trap_not_used)
DECLARE_TRAP_HANDLER(irq)
DECLARE_TRAP_HANDLER(fiq)

void do_data_abort(struct trap_frame *tf)
{
    u32 addr;
    get_dfar(&addr);
    printk("Failed to access [0x%08x]\n", addr);
    show_tf(tf);
}
