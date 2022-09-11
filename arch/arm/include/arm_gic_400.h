#ifndef __TINY_KERNEL_ARM_GIC_400_H__
#define __TINY_KERNEL_ARM_GIC_400_H__

#include "arm_iomap.h"

#define GIC_BASE ((MMIO_BASE)+0xB000)

#define GICD_BASE ((GIC_BASE)+0x1000)
#define GICC_BASE ((GIC_BASE)+0x2000)

#define GICD_CTLR       ((volatile u32*)(GICD_BASE+0x0000))
#define GICD_TYPER      ((volatile u32*)(GICD_BASE+0x0004))
#define GICD_IGROUPR    ((volatile u32*)(GICD_BASE+0x0080))
#define GICD_ISENABLER  ((volatile u32*)(GICD_BASE+0x0100))
#define GICD_ICENABLER  ((volatile u32*)(GICD_BASE+0x0180))
#define GICD_ICPENDR    ((volatile u32*)(GICD_BASE+0x0280))
#define GICD_ISACTIVER  ((volatile u32*)(GICD_BASE+0x0300))
#define GICD_ICACTIVER  ((volatile u32*)(GICD_BASE+0x0380))
#define GICD_IPRIORITYR ((volatile u32*)(GICD_BASE+0x0400))
#define GICD_ITARGETSR  ((volatile u32*)(GICD_BASE+0x0800))
#define GICD_ICFGR      ((volatile u32*)(GICD_BASE+0x0C00))
#define GICD_SGIR       ((volatile u32*)(GICD_BASE+0x0F00))
#define GICD_CPENGSGIR  ((volatile u32*)(GICD_BASE+0x0F10))
#define GICD_ID2        ((volatile u32*)(GICD_BASE+0x0FE8))

#define GICC_CTLR       ((volatile u32*)(GICC_BASE+0x0000))
#define GICC_PMR        ((volatile u32*)(GICC_BASE+0x0004))
#define GICC_IAR        ((volatile u32*)(GICC_BASE+0x000C))
#define GICC_EOIR       ((volatile u32*)(GICC_BASE+0x0010))
#define GICC_IIDR       ((volatile u32*)(GICC_BASE+0x00FC))
#define GICC_DIR        ((volatile u32*)(GICC_BASE+0x1000))

/* for GICD_TYPER */
#define GICD_TYPER_IRQ_NUM_MASK 0xF
#define GICC_IAR_IRQ_NUM_MASK 0x3FF

#define GIC_IS_SOFT_IRQ(irqno) (irqno < 16)

void gic_400_init();
struct trap_frame;
int get_trap_no(struct trap_frame * tf);
void gic_400_irq_disable(int irqno);
void gic_400_irq_enable(int irqno);
void gic_400_end();

#endif
