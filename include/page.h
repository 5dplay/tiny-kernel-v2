#ifndef __TINY_KERNEL_PAGE_H__
#define __TINY_KERNEL_PAGE_H__

#define PAGE_MASK (~(PAGE_SIZE-1))
#define PAGE_ALIGN_CEIL(addr) (((addr)+PAGE_SIZE-1)&PAGE_MASK)
#define PAGE_ALIGN_FLOOR(addr) ((addr)&PAGE_MASK)

#endif
