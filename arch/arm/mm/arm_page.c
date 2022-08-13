/*
    arm处理页管理相关的逻辑
    FIXME: 多架构时候如何处理?
*/
#include "list.h"
#include "mm.h"
#include "type.h"
#include "common.h"
#include "memlayout.h"
#include "string.h"
#if 0
#include "fs.h"
#endif

#include "arm_mm.h"
#include "arm_page.h"

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

    if (*pde_val & PDE_DEF_FLAGS) {
        pg_tbl = (u32 *)phy_to_virt(PDE_ADDR(*pde_val));
    } else {
        if (!create)
            return NULL;

        /* 这里实际上只要0x400就够了，但是实现相关代码较为麻烦，这里便宜行事 */
        pg_tbl = page_alloc();

        //printk("pde_idx = 0x%x, alloc %p\n", pde_idx, pg_tbl);
        if (NULL == pg_tbl) {
            printk("failed to alloc page table!\n");
            return NULL;
        }

        memset(pg_tbl, 0x0, PAGE_SIZE);
        *pde_val = virt_to_phy((uaddr)pg_tbl) | (PDE_DEF_FLAGS);
    }

    return &pg_tbl[PTE_IDX(vaddr)];
}
static int s_pg_map_helper(struct arm_vmm *this, uaddr pg_dir, uaddr vaddr, uaddr paddr, uint perm_flags)
{
    /*
        step 1: get pg dir entry, if not present, then alloc a pg table
        step 2: access pg table, check sanity, map virt to phy
    */
    u32 *pte_val;
    pte_val = s_get_pg_pte((u32 *)pg_dir, vaddr, 1);

    if (NULL == pte_val) {
        printk("failed to get page table!\n");
        return -1;
    }

    if (*pte_val & PTE_DEF_FLAGS) {
        printk("pg_dir=%x, virt %x already map to %x\n", pg_dir, vaddr, PTE_ADDR((uintptr_t)*pte_val));
        return -1;
    }

    *pte_val = paddr | perm_flags | PTE_DEF_FLAGS;
    return 0;
}

TK_STATUS arm_page_map(struct arm_vmm *this, struct page_map_info *info)
{
    uaddr pg_dir, vaddr, paddr, paddr_end;
    uint size, flags;

    pg_dir = info->pg_dir;
    vaddr = info->vaddr;
    paddr = info->paddr;
    size = info->size;
    flags = info->flags;

    paddr_end = paddr + size;

    printk("%s: map %x -> %x, size = 0x%x\n", __func__, vaddr, paddr, size);

    while (paddr < paddr_end) {
        if (s_pg_map_helper(this, pg_dir, vaddr, paddr, flags) < 0) {
            panic("%s: failed to map %x -> %x\n", __func__, vaddr, paddr);
        }

        paddr += PAGE_SIZE;
        vaddr += PAGE_SIZE;
    }

    return TK_STATUS_SUCCESS;
}

int __arm_vm_resize_dec(struct arm_vmm *this, uaddr pg_dir, uint old_sz, uint new_sz)
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
        } else if (*pte_val & PTE_DEF_FLAGS) {
            mem = PTE_ADDR(*pte_val);

            page_free((void *)phy_to_virt(mem));

            *pte_val = 0;
        }
    }

    return new_sz;
}

#if 0
//此外这里仍然做的一件事就是将用户的虚拟地址重新解释成内核的虚拟地址. 这里是否可以抽象出来呢?
int arm_page_prog_load(struct arm_vmm *this, struct vm_prog_load_params *params)
{
    u32 *pte_val;
    uaddr pg_dir, vaddr;
    uaddr pa, va;
    uint i, n, off, cnt;

    pg_dir = params->pg_dir;
    vaddr = params->usr_va;
    off = params->off;

    for (i = 0; i < params->size; i += PAGE_SIZE, vaddr += PAGE_SIZE) {
        pte_val = s_get_pg_pte((struct allocator *)&this->m_bootm_alloc,
                               (u32 *)pg_dir, vaddr, 0);

        if ((NULL == pte_val) && !(*pte_val & PAGING_BIT_P))
            panic("%s: vaddr = %p not exist.\n", vaddr);

        n = min(params->size - i, PAGE_SIZE);
        pa = PTE_ADDR(*pte_val);
        va = phy_to_virt(pa);
        cnt = readi(params->ip, (u8 *)va, off + i, n);

        if (cnt != n)
            return -1;
    }

    return 0;
}
#endif

TK_STATUS arm_page_free_pg_dir(struct arm_vmm *this, uaddr pg_dir)
{
    int i;
    u32 *pde;
    //FIXME: 异常处理
    __arm_vm_resize_dec(this, pg_dir, KERNEL_BASE, 0);

    pde = (u32 *)pg_dir;

    for (i = 0; i < NPDENTRIES; i++) {
        if (pde[i] & PDE_DEF_FLAGS) {
            page_free((void *)phy_to_virt(PDE_ADDR(pde[i])));
        }
    }

    page_free_v2(pde, 2);

    return TK_STATUS_SUCCESS;
}
#if 0
int arm_page_clone_pg_dir(struct arm_vmm *this, uaddr dst, uaddr src, uaddr va)
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
            arm_page_map(this, dst, start, virt_to_phy(new_vaddr), PAGE_SIZE, flags);
        }

    }

    return 0;
}
#endif
