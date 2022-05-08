#ifndef __TINY_KERNEL_MEMLAYOUT_H__
#define __TINY_KERNEL_MEMLAYOUT_H__

#define PHY_0M 0x000000
#define PHY_1M 0x100000
#define PHY_4M 0x400000
#define PHY_8M 0x800000
#define PHY_TOP 0x20000000 //实际使用的最高内存物理地址512MB处

#define KERNEL_BASE 0x80000000
#define phy_to_virt(addr) (addr+KERNEL_BASE)
#define virt_to_phy(addr) (addr-KERNEL_BASE)

#define BOOT_KERNEL_STACK_SIZE 0x8000
#endif
