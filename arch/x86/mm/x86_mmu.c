#include "mm.h"
#include "vm.h"
#include "memlayout.h"

#include "x86_asm.h"
#include "x86_mm.h"
#include "x86_seg.h"
#include "x86_page.h"

struct x86_vmm g_vm;

vmm *arch_mmu_init()
{
    seg_init(&g_vm);

    return (vmm *)&g_vm;
}

uaddr vm_alloc_pg_dir(vmm *vm)
{
    uaddr pg_dir;

    pg_dir = (uaddr)(g_mm_initialized ? page_alloc() : bootm_alloc());

    return pg_dir;
}

void vm_free_pg_dir(vmm *vm, uaddr pg_dir)
{
    x86_page_free_pg_dir((struct x86_vmm *)vm, pg_dir);
}

TK_STATUS vm_map(vmm *vm, uaddr pg_dir, uaddr v_addr, uaddr p_addr,
                 uint size, uint perm_flags)
{
    struct x86_vmm *this;
    uint x86_perm_flags;

    x86_perm_flags = 0;
    this = (struct x86_vmm *)vm;

    if (perm_flags & VM_PERM_USER)
        x86_perm_flags |= 0x4;

    if (perm_flags & VM_PERM_WRITE)
        x86_perm_flags |= 0x2;

    return x86_page_map(this, pg_dir, v_addr, p_addr, size, x86_perm_flags);
}

void vm_reload(vmm *vm, uaddr pg_dir)
{
    lcr3(virt_to_phy(pg_dir));
}
