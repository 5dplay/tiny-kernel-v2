#ifndef __TINY_KERNEL_X86_PIC_H__
#define __TINY_KERNEL_X86_PIC_H__

#include "type.h"

void pic_init();
void pic_end();
void pic_irq_enable(int irqno);
void pic_irq_disable(int irqno);

#endif
