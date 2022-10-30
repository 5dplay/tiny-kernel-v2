#include "mm.h"
#include "vm.h"
#include "type.h"
#include "common.h"
#include "memlayout.h"
#include "string.h"
#include "indexfs.h"

#include "x86_mm.h"
#include "x86_page.h"
#include "x86_seg.h"

//rewriten page map
/**
* @brief get page table using virtual address
*
* @param [in]pg_dir page directory
* @param [in]virt virtual address
*
* @return failure => == NULL, success => != NULL
*/
static u32* s_get_pg_pte(u32 *pg_dir, uaddr vaddr, int create)
{
    u32 pde_idx;
    u32 *pde_val;
    u32 *pg_tbl;

    pde_idx = PDE_IDX(vaddr);
    pde_val = &pg_dir[pde_idx];

    if (*pde_val & PAGING_BIT_P) {
        pg_tbl = (u32 *)phy_to_virt(PDE_ADDR(*pde_val));
    } else {
        //这里需要保证页对齐
        if (!create)
            return NULL;

        pg_tbl = page_alloc();

        if (NULL == pg_tbl) {
            printk("failed to alloc page table!\n");
            return NULL;
        }

        memset(pg_tbl, 0x0, PAGE_SIZE);
        //这里其实是过于宽松的的权限了,但是会在后面具体页表项做严格的限制
        *pde_val = virt_to_phy((uaddr)pg_tbl) | (0x7);
    }

    return &pg_tbl[PTE_IDX(vaddr)];
}
static int s_pg_map_helper(struct x86_vmm *this, uaddr pg_dir, uaddr vaddr, uaddr paddr, uint perm_flags)
{
    /*
        step 1: get pg dir entry, if not present, then alloc a pg table
        step 2: access pg table, check sanity, map virt to phy
    */
    u32 *pte_val;

    pte_val = s_get_pg_pte((u32 *)pg_dir, vaddr, 1);

    if (NULL == pte_val) {
        printk("failed to get page table!");
        return -1;
    }

    if (*pte_val & PAGING_BIT_P) {
        printk("pg_dir=%x, virt %x already map to %x\n", pg_dir, vaddr, PTE_ADDR((uintptr_t)*pte_val));
        return -1;
    }

    *pte_val = paddr | perm_flags | PAGING_BIT_P;

    return 0;
}

TK_STATUS x86_page_map(struct x86_vmm *this, uaddr pg_dir, uaddr vaddr, uaddr paddr, uint size, uint flags)
{
    uaddr paddr_end;

    paddr_end = paddr + size;

    while (paddr < paddr_end) {
        if (s_pg_map_helper(this, pg_dir, vaddr, paddr, flags) < 0) {
            panic("%s: failed to map %x -> %x\n", __func__, vaddr, paddr);
        }

        paddr += PAGE_SIZE;
        vaddr += PAGE_SIZE;
    }

    return TK_STATUS_SUCCESS;
}

int __x86_vm_resize_inc(struct x86_vmm *this, uaddr pg_dir, uint old_sz, uint new_sz)
{
    uaddr start, mem;

    if (new_sz >= KERNEL_BASE)
        return 0;

    if (old_sz > new_sz)
        return old_sz;

    start = PAGE_ALIGN_CEIL(old_sz);

    for (; start < new_sz; start += PAGE_SIZE) {
        mem = (uaddr)page_alloc();

        if (mem == 0)
            panic("%s: oom, TBD.\n", __func__);

        vm_map((vmm *)this, pg_dir, start, virt_to_phy(mem), PAGE_SIZE, VM_PERM_WRITE | VM_PERM_USER);
    }

    return new_sz;
}


int __x86_vm_resize_dec(struct x86_vmm *this, uaddr pg_dir, uint old_sz, uint new_sz)
{
    uaddr start, mem;
    u32 *pte_val;

    if (new_sz >= old_sz)
        return old_sz;

    start = PAGE_ALIGN_CEIL(new_sz);

    for (; start < old_sz; start += PAGE_SIZE) {
        pte_val = s_get_pg_pte((u32 *)pg_dir, start, 0);

        if (pte_val == NULL) {
            //跳到下一个页表
            start = GEN_PDE_ADDR(PDE_IDX(start) + 1, 0, 0) - PAGE_SIZE;
        } else if (*pte_val & PAGING_BIT_P) {
            mem = PTE_ADDR(*pte_val);

            page_free((void *)phy_to_virt(mem));

            *pte_val = 0;
        }
    }

    return new_sz;
}

//此外这里仍然做的一件事就是将用户的虚拟地址重新解释成内核的虚拟地址. 这里是否可以抽象出来呢?
int x86_page_prog_load(struct x86_vmm *this, struct vm_prog_load_params *params)
{
    u32 *pte_val;
    uaddr pg_dir, vaddr;
    uaddr pa, va;
    uint i, n, off, cnt;
    struct inode *ip;

    pg_dir = params->pg_dir;
    vaddr = params->usr_va;
    off = params->off;
    ip = params->ip;

    for (i = 0; i < params->size; i += PAGE_SIZE, vaddr += PAGE_SIZE) {
        pte_val = s_get_pg_pte((u32 *)pg_dir, vaddr, 0);

        if ((NULL == pte_val) && !(*pte_val & PAGING_BIT_P))
            panic("%s: vaddr = %p not exist.\n", vaddr);

        n = min(params->size - i, PAGE_SIZE);
        pa = PTE_ADDR(*pte_val);
        va = phy_to_virt(pa);
        cnt = data_readi(ip->superb, ip, (u8 *)va, off + i, n);

        if (cnt != n)
            return -1;
    }

    return 0;
}

TK_STATUS x86_page_free_pg_dir(struct x86_vmm *this, uaddr pg_dir)
{
    int i;
    u32 *pde;
    //FIXME: 异常处理
    __x86_vm_resize_dec(this, pg_dir, KERNEL_BASE, 0);

    pde = (u32 *)pg_dir;

    for (i = 0; i < NPDENTRIES; i++) {
        if (pde[i] & PAGING_BIT_P) {
            page_free((void *)phy_to_virt(PDE_ADDR(pde[i])));
        }
    }

    page_free(pde);

    return TK_STATUS_SUCCESS;
}

#if 0
int x86_page_clone_pg_dir(struct x86_vmm *this, uaddr dst, uaddr src, uaddr va)
{
    uaddr start, paddr, new_vaddr;
    u32 *pte_val, flags;

    start = 0;

    for (; start < va; start += PAGE_SIZE) {
        pte_val = s_get_pg_pte((u32 *)src, start, 0);

        if (pte_val == NULL) {
            //跳到下一个页表
            start = GEN_PDE_ADDR(PDE_IDX(start) + 1, 0, 0) - PAGE_SIZE;
        } else if (*pte_val & PAGING_BIT_P) {
            paddr = PTE_ADDR(*pte_val);
            flags = PTE_FLAGS(*pte_val);
            new_vaddr = (uaddr)(g_mm_initialized ? page_alloc() : bootm_alloc());
            memcpy((void *)new_vaddr, (void *)phy_to_virt(paddr), PAGE_SIZE);
            x86_page_map(this, dst, start, virt_to_phy(new_vaddr), PAGE_SIZE, flags);
        }

    }

    return 0;
}
#endif
