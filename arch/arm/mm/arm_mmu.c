#include "common.h"
#include "memlayout.h"
#include "mm.h"
#include "type.h"
#include "vm.h"
#include "arm_asm.h"
#include "arm_mm.h"
#include "arm_page.h"

struct arm_vmm g_vm;

vmm *arch_mmu_init()
{
    /* boot.S中已经将mmu相关的给初始化好了，没必要再设置一遍 */
    return (vmm *)&g_vm;
}

uaddr vm_alloc_pg_dir(vmm *vm)
{
    uaddr pg_dir;

    /* arm32 页目录需要16k，并且必须是按照16k对齐的 */
    pg_dir = (uaddr)page_alloc_v2(2);

    return pg_dir;
}

void vm_free_pg_dir(vmm *vm, uaddr pg_dir)
{
    arm_page_free_pg_dir((struct arm_vmm *)vm, pg_dir);
}

TK_STATUS vm_map(vmm *vm, uaddr pg_dir, uaddr v_addr, uaddr p_addr,
                 uint size, uint perm_flags)
{
    struct arm_vmm *this;
    struct page_map_info info;
    uint arm_perm_flags;

    arm_perm_flags = 0;
    this = (struct arm_vmm *)vm;

    if (perm_flags & VM_PERM_USER)
        arm_perm_flags |= 0x20;
    else
        arm_perm_flags |= 0x10;

    if (!(perm_flags & VM_PERM_WRITE))
        arm_perm_flags |= 0x100;

    info.pg_dir = pg_dir;
    info.vaddr = v_addr;
    info.paddr = p_addr;
    info.size = size;
    info.flags = arm_perm_flags;
    return arm_page_map(this, &info);
}

void vm_reload(vmm *vm, uaddr pg_dir)
{
    mcr_ttbr0(virt_to_phy(pg_dir));
    flush_tlb();
    return ;
}

uint vm_resize(vmm *vm, uaddr pg_dir, uint old_sz, uint new_sz)
{
    if (new_sz > old_sz)
        return __arm_vm_resize_inc((struct arm_vmm *)vm, pg_dir, old_sz, new_sz);
    else
        return __arm_vm_resize_dec((struct arm_vmm *)vm, pg_dir, old_sz, new_sz);
}

TK_STATUS vm_prog_load(vmm *vm, struct vm_prog_load_params *params)
{
    return arm_page_prog_load((struct arm_vmm *)vm, params);
}
