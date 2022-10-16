#ifndef __TINY_KERNEL_ARM_TRAP_H__
#define __TINY_KERNEL_ARM_TRAP_H__

#ifndef __ASSEMBLER__
#include "type.h"
#endif

#define S_FRAME_SIZE    72

#define S_OLD_R0        68
#define S_PSR           64
#define S_PC            60
#define S_LR            56
#define S_SP            52

#define S_IP            48
#define S_FP            44
#define S_R10           40
#define S_R9            36
#define S_R8            32
#define S_R7            28
#define S_R6            24
#define S_R5            20
#define S_R4            16
#define S_R3            12
#define S_R2            8
#define S_R1            4
#define S_R0            0

#define MODE_MASK 0x1f
#define MODE_SVC 0x13
#define MODE_USER 0x10
#define I_BIT 0x80

#define IRQ_TIMER0   0
#define IRQ_TIMER1   1
#define IRQ_TIMER2   2
#define IRQ_TIMER3   3
#define IRQ_AUX     29
#define IRQ_MAX     64

#ifndef __ASSEMBLER__
struct trap_frame {
    u32 r0;
    u32 r1;
    u32 r2;
    u32 r3;
    u32 r4;
    u32 r5;
    u32 r6;
    u32 r7;
    u32 r8;
    u32 r9;
    u32 r10;
    u32 fp;
    u32 ip;

    u32 sp;
    u32 lr;
    u32 pc;
    u32 cpsr;
    u32 org_r0;
};

#include "arm_iomap.h"

#define IRQ_REG_BASE            ((MMIO_BASE)+0xB000)
#define IRQ_REG_BASE_PEND       ((volatile u32*)((IRQ_REG_BASE)+0x200))
#define IRQ_REG_PEND1           ((volatile u32*)((IRQ_REG_BASE)+0x204))
#define IRQ_REG_PEND2           ((volatile u32*)((IRQ_REG_BASE)+0x208))
#define IRQ_REG_FIQ_CTRL        ((volatile u32*)((IRQ_REG_BASE)+0x20C))
#define IRQ_REG_EN_IRQ1         ((volatile u32*)((IRQ_REG_BASE)+0x210))
#define IRQ_REG_EN_IRQ2         ((volatile u32*)((IRQ_REG_BASE)+0x214))
#define IRQ_REG_EN_BASIC_IRQ    ((volatile u32*)((IRQ_REG_BASE)+0x218))
#define IRQ_REG_DIS_IRQ1        ((volatile u32*)((IRQ_REG_BASE)+0x21C))
#define IRQ_REG_DIS_IRQ2        ((volatile u32*)((IRQ_REG_BASE)+0x220))
#define IRQ_REG_DIS_BASIC_IRQ   ((volatile u32*)((IRQ_REG_BASE)+0x224))

#define CPSR_DIS_IRQ_BIT (1<<7)
#define CPSR_DIS_FIQ_BIT (1<<6)

#endif

#endif
