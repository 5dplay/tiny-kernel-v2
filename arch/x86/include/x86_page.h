#ifndef __TINY_KERNEL_X86_PAGE_H__
#define __TINY_KERNEL_X86_PAGE_H__

#include "page.h"

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


#ifndef __ASSEMBLER__
#include "type.h"

struct x86_vmm;
/**
* @brief x86页虚拟地址映射
*
* @param this x86内存管理器
* @param pg_dir 页目录
* @param vaddr 起始虚拟地址
* @param paddr 对应映射的起始物理地址
* @param size 内存块大小，单位——字节
* @param flags 权限
*
* @return TK_STATUS
*/
TK_STATUS x86_page_map(struct x86_vmm *this, uaddr pg_dir, uaddr vaddr, uaddr paddr, uint size, uint flags);

/**
* @brief 释放页目录占用的所有空间
*
* @param this x86内存管理器
* @param pg_dir 页目录
*
* @return
*/
TK_STATUS x86_page_free_pg_dir(struct x86_vmm *this, uaddr pg_dir);

struct vm_prog_load_params;
TK_STATUS x86_page_prog_load(struct x86_vmm *this, struct vm_prog_load_params *params);

int __x86_vm_resize_inc(struct x86_vmm *this, uaddr pg_dir, uint old_sz, uint new_sz);
int __x86_vm_resize_dec(struct x86_vmm *this, uaddr pg_dir, uint old_sz, uint new_sz);
#endif /* __ASSEMBLER__ */

#endif
