#include "common.h"
#include "arm_gic_400.h"
#include "arm_asm.h"

static u32 max_irq_line_num = 0;
static u32 max_irq_num = 0;

void gic_400_init()
{
    u32 i;
    volatile u32 *ptr0, *ptr1;

    /* copy from FreeNOS, see lib/libarch/arm/ARMGenericInterrupt.cpp */
    max_irq_line_num = ((*GICD_TYPER) & GICD_TYPER_IRQ_NUM_MASK) + 1;
    max_irq_num = 32 * max_irq_line_num;

    printk("max_irq_num = %u\n", max_irq_num);

    /* Set all interrupts in group 0 and disable all interrupts */
    ptr0 = GICD_IGROUPR;
    ptr1 = GICD_ICENABLER;

    for (i = 0; i < max_irq_line_num; i++, ptr0++, ptr1++) {
        *ptr0 = 0;
        *ptr1 = ~0;
    }

    /* Set interrupt configuration to level triggered (2-bits) */
    ptr0 = GICD_ICFGR;

    /* NOTE: 这里原作者估计写错了，但是恰好这块区域足够大，因此没出问题 */
    for (i = 0; i < (max_irq_num / 16); i++, ptr0++)
        *ptr0 = 0;

    /* Set interrupt priority */
    ptr0 = GICD_IPRIORITYR;

    for (i = 0; i < (max_irq_num / 4); i++, ptr0++)
        *ptr0 = 0xA0A0A0A0;

    /* All interrupts are assigned to core0.
     * Ignore the first 32 PPIs, which are local and banked per core.
     */
    ptr0 = GICD_ITARGETSR;

    for (i = 0; i < (max_irq_num / 4); i++, ptr0++)
        *ptr0 = 0x01010101;

    /* Enable all groups */
    *GICD_CTLR = 0x3;

    /* Set minimum priority level and enable all groups */
    *GICC_PMR = 0xF0;
    *GICC_CTLR = 0x3;
}

void gic_400_irq_disable(int irqno)
{
    volatile u32 *ptr;

    ptr = GICD_ICENABLER;

    if (!GIC_IS_SOFT_IRQ(irqno) && (irqno < max_irq_num))
        ptr[irqno / 32] = (1 << (irqno % 32)); /* 直接写该位即可，覆盖其他位无影响 */
}

void gic_400_irq_enable(int irqno)
{
    volatile u32 *ptr;

    ptr = GICD_ISENABLER;

    if (!GIC_IS_SOFT_IRQ(irqno) && (irqno < max_irq_num))
        ptr[irqno / 32] = (1 << (irqno % 32)); /* 直接写该位即可，覆盖其他位无影响 */
}

static u32 cur_irq_ack = 0;
int get_trap_no(struct trap_frame * tf)
{
    volatile u32 *ptr;

    ptr = GICC_IAR;

    cur_irq_ack = *ptr;

    return cur_irq_ack & GICC_IAR_IRQ_NUM_MASK;
}

void gic_400_end()
{
    volatile u32 *ptr;

    ptr = GICC_EOIR;
    *ptr = cur_irq_ack;

    ptr = GICC_DIR;
    *ptr = cur_irq_ack;
}
