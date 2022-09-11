#include "memlayout.h"
#include "common.h"
#include "trap.h"
#include "mm.h"
#include "type.h"
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
/* DECLARE_TRAP_HANDLER(irq) */
/* DECLARE_TRAP_HANDLER(fiq) */

void do_data_abort(struct trap_frame *tf)
{
    u32 addr;
    get_dfar(&addr);
    printk("Failed to access [0x%08x]\n", addr);
    show_tf(tf);
}

TK_STATUS irq_init()
{
    volatile u32 *ptr;

    /* disable all interrupt */
    ptr = IRQ_REG_DIS_IRQ1;
    *ptr = 0xFFFFFFFF;
    ptr = IRQ_REG_DIS_IRQ2;
    *ptr = 0xFFFFFFFF;
    ptr = IRQ_REG_DIS_BASIC_IRQ;
    *ptr = 0xFFFFFFFF;

    printk("dis_irq1 = %08x, dis_irq2 = %08x, dis_basic_irq = %08x\n",
           *IRQ_REG_DIS_IRQ1, *IRQ_REG_DIS_IRQ2, *IRQ_REG_DIS_BASIC_IRQ);

    return TK_STATUS_SUCCESS;
}

trap_handler g_irq_handler[IRQ_MAX];

int get_trap_no_bcm2835(struct trap_frame *tf)
{
    int i;
    u32 val;

    val = *IRQ_REG_BASE_PEND;

    for (i = 0; i < 4; i++)
        if (val & (1 << i))
            return i;

    if (val & (1 << 8)) {
        val = *IRQ_REG_PEND1;

        for (i = 0; i < 32; i++)
            if (val & (1 << i))
                return i;
    } else if (val & (1 << 9)) {
        val = *IRQ_REG_PEND2;

        for (i = 0; i < 32; i++)
            if (val & (1 << i))
                return i + 32;
    }

    return -1;
}
void do_irq(struct trap_frame *tf)
{
    int irq_no;

    irq_no = get_trap_no_bcm2835(tf);

    if (0 < irq_no && irq_no < IRQ_MAX && g_irq_handler[irq_no])
        g_irq_handler[irq_no](tf);
    else
        printk("unregister irq no == %d\n", irq_no);

    irq_end();
}

void do_fiq(struct trap_frame *tf)
{
    u32 fiq;

    fiq = *IRQ_REG_FIQ_CTRL;
    printk("fiq = %08x\n", fiq);

    while (1);
}

/**
* @brief 为相关的中断号irqno注册处理函数
*
* @param irqno 中断号
* @param func 中断处理函数
*/
void irq_register(int irqno, trap_handler func)
{
    if (irqno < 0 || irqno >= IRQ_MAX) {
        printk("trap range [0, %d)", IRQ_MAX);
        return ;
    }

    if (NULL != g_irq_handler[irqno]) {
        printk("irq %x was already registered\n", irqno);
        return ;
    }

    g_irq_handler[irqno] = func;
}

/**
* @brief 注销相关的中断号irqno相关的处理函数
*
* @param irqno 中断号
* @param func 中断处理函数
*/
void irq_unregister(int irqno, trap_handler func)
{
    if (irqno < 0 || irqno >= IRQ_MAX) {
        printk("trap range [0, %d)", IRQ_MAX);
        return ;
    }

    g_irq_handler[irqno] = NULL;
}

void irq_enable(int irqno)
{
    volatile u32 *ptr;

    if (irqno < 16) {
        ptr = IRQ_REG_EN_BASIC_IRQ;
        *ptr = (1 << irqno);
    } else if (irqno < 32) {
        ptr = IRQ_REG_EN_IRQ1;
        *ptr = (1 << irqno);
    } else {
        ptr = IRQ_REG_EN_IRQ2;
        *ptr = (1 << (irqno - 32));
    }

}

void irq_disable(int irqno)
{
    volatile u32 *ptr;

    if (irqno < 16) {
        ptr = IRQ_REG_DIS_BASIC_IRQ;
        *ptr = (1 << irqno);
    } else if (irqno < 32) {
        ptr = IRQ_REG_DIS_IRQ1;
        *ptr = (1 << irqno);
    } else {
        ptr = IRQ_REG_DIS_IRQ2;
        *ptr = (1 << (irqno - 32));
    }

}

void irq_end()
{

}

#define CPSR_IRQ_BIT (CPSR_DIS_IRQ_BIT | CPSR_DIS_FIQ_BIT)

void disable_trap()
{
    u32 cpsr;

    get_cpsr(&cpsr);
    cpsr |= CPSR_IRQ_BIT;
    set_cpsr(cpsr);
}

void enable_trap()
{
    u32 cpsr;

    get_cpsr(&cpsr);
    cpsr &= ~CPSR_IRQ_BIT;
    set_cpsr(cpsr);
}
