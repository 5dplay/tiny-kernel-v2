#include "common.h"
#include "x86_pic.h"
#include "x86_asm.h"

#define MASTER_PIC 0x20
#define SLAVE_PIC 0xA0

#define MASTER_PIC_CMD MASTER_PIC
#define SLAVE_PIC_CMD SLAVE_PIC
#define MASTER_PIC_DATA (MASTER_PIC+1)
#define SLAVE_PIC_DATA (SLAVE_PIC+1)

#define DEFAULT_MASTER_MASK 0xFB
#define DEFAULT_SLAVE_MASK 0xFF


u8 master_pic_mask = DEFAULT_MASTER_MASK;
u8 slave_pic_mask = DEFAULT_SLAVE_MASK;

void pic_init()
{
    //ICW1
    outb(MASTER_PIC_CMD, 0x11);
    outb(SLAVE_PIC_CMD, 0x11);

    //ICW2
    outb(MASTER_PIC_DATA, 0x20);/*IRQ0 -> 0x20*/
    outb(SLAVE_PIC_DATA, 0x28);/*IRQ8 -> 0x28*/

    //ICW3
    outb(MASTER_PIC_DATA, 0x4);/*master*/
    outb(SLAVE_PIC_DATA, 0x2);/*slave*/

    //ICW4
    outb(MASTER_PIC_DATA, 0x1);/*both use 8086 mode*/
    outb(SLAVE_PIC_DATA, 0x1);

    outb(MASTER_PIC_DATA, DEFAULT_MASTER_MASK);
    outb(SLAVE_PIC_DATA, DEFAULT_SLAVE_MASK); /*enable irq2 for slave 8259A*/
}

void pic_end()
{
    outb(SLAVE_PIC, 0x20);
    outb(MASTER_PIC, 0x20);
}

void pic_irq_disable(int irqno)
{
    uint8_t mask;
    uint32_t flags;

    mask = 1 << (irqno & 0x7);
    flags = save_flags();
    cli();

    if (irqno < 8) {
        //master
        master_pic_mask |= mask;
        outb(master_pic_mask, MASTER_PIC_DATA);
    } else {
        //slave
        slave_pic_mask |= mask;
        outb(slave_pic_mask, SLAVE_PIC_DATA);
    }

    restore_flags(flags);
}

void pic_irq_enable(int irqno)
{
    uint8_t mask;
    uint32_t flags;
    mask = ~(1 << (irqno & 0x7));
    flags = save_flags();
    cli();

    if (irqno < 8) {
        //master
        master_pic_mask &= mask;
        outb(MASTER_PIC_DATA, master_pic_mask);
    } else {
        //slave
        slave_pic_mask &= mask;
        outb(SLAVE_PIC_DATA, slave_pic_mask);
    }

    restore_flags(flags);
}
