#ifndef __TINY_KERNEL_PAGE_H__
#define __TINY_KERNEL_PAGE_H__

#define PAGE_SIZE 0x1000
#define PAGE_MASK (~(PAGE_SIZE-1))
#define PAGE_ALIGN_CEIL(addr) (((addr)+PAGE_SIZE-1)&PAGE_MASK)
#define PAGE_ALIGN_FLOOR(addr) ((addr)&PAGE_MASK)

#define PDE_SHIFT 22
#define PDE_IDX(addr) (((addr)>>PDE_SHIFT)&0x3FF)
#define PDE_ADDR(val) ((val)&PAGE_MASK)

#define PTE_SHIFT 12
#define PTE_IDX(addr) (((addr)>>PTE_SHIFT)&0x3FF)
#define PTE_ADDR(val) ((val)&PAGE_MASK)
#define PTE_FLAGS(val) ((val)&(PAGE_SIZE-1))

//pde idx | pte idx | offset
#define GEN_PDE_ADDR(d, t, o) ((uint)((d) << PDE_SHIFT | (t) << PTE_SHIFT | (o)))
#define NPDENTRIES 1024 // # directory entries per page directory

#define PAGING_BIT_P 0x1

#endif
