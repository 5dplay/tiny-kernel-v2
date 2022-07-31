#include "common.h"
#include "mm.h"
#include "vm.h"
#include "arm_mm.h"

struct arm_vmm g_vm;

vmm *arch_mmu_init()
{
    /* boot.S中已经将mmu相关的给初始化好了，没必要再设置一遍 */
    return (vmm *)&g_vm;
}

uaddr vm_alloc_pg_dir(vmm *vm)
{
    uaddr pg_dir;

    #define TTBR0_ALIGN_SIZE 0x4000
    pg_dir = (uaddr)page_align_alloc(TTBR0_ALIGN_SIZE);

    return pg_dir;
}

void vm_free_pg_dir(vmm *vm, uaddr pg_dir)
{
    panic("TO BE DONE\n");
}

TK_STATUS vm_map(vmm *vm, uaddr pg_dir, uaddr v_addr, uaddr p_addr,
                 uint size, uint perm_flags)
{
    struct arm_vmm *this;
    uint x86_perm_flags;

    x86_perm_flags = 0;
    this = (struct arm_vmm *)vm;

    if (perm_flags & VM_PERM_USER)
        x86_perm_flags |= 0x4;

    if (perm_flags & VM_PERM_WRITE)
        x86_perm_flags |= 0x2;

    return arm_page_map(this, pg_dir, v_addr, p_addr, size, x86_perm_flags);
}
