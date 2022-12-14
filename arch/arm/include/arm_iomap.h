#ifndef __TINY_KERNEL_ARM_IOMAP_H__
#define __TINY_KERNEL_ARM_IOMAP_H__
/*
 * Copyright (C) 2018 bzt (bztsrc@github)
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 */
#include "memlayout.h"
#include "page.h"
#include "arm_page.h"
#include "type.h"

#define MMIO_BASE       phy_to_virt(MMIO_BASE_PHY)

#define GPFSEL0         ((volatile u32*)(MMIO_BASE+0x00200000))
#define GPFSEL1         ((volatile u32*)(MMIO_BASE+0x00200004))
#define GPFSEL2         ((volatile u32*)(MMIO_BASE+0x00200008))
#define GPFSEL3         ((volatile u32*)(MMIO_BASE+0x0020000C))
#define GPFSEL4         ((volatile u32*)(MMIO_BASE+0x00200010))
#define GPFSEL5         ((volatile u32*)(MMIO_BASE+0x00200014))
#define GPSET0          ((volatile u32*)(MMIO_BASE+0x0020001C))
#define GPSET1          ((volatile u32*)(MMIO_BASE+0x00200020))
#define GPCLR0          ((volatile u32*)(MMIO_BASE+0x00200028))
#define GPLEV0          ((volatile u32*)(MMIO_BASE+0x00200034))
#define GPLEV1          ((volatile u32*)(MMIO_BASE+0x00200038))
#define GPEDS0          ((volatile u32*)(MMIO_BASE+0x00200040))
#define GPEDS1          ((volatile u32*)(MMIO_BASE+0x00200044))
#define GPHEN0          ((volatile u32*)(MMIO_BASE+0x00200064))
#define GPHEN1          ((volatile u32*)(MMIO_BASE+0x00200068))
#define GPPUD           ((volatile u32*)(MMIO_BASE+0x00200094))
#define GPPUDCLK0       ((volatile u32*)(MMIO_BASE+0x00200098))
#define GPPUDCLK1       ((volatile u32*)(MMIO_BASE+0x0020009C))

#endif
