#include "mm.h"
#include "type.h"
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

    pg_dir = (uaddr)page_alloc();

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

uint vm_resize(vmm *vm, uaddr pg_dir, uint old_sz, uint new_sz)
{
    if (new_sz > old_sz)
        return __x86_vm_resize_inc((struct x86_vmm *)vm, pg_dir, old_sz, new_sz);
    else
        return __x86_vm_resize_dec((struct x86_vmm *)vm, pg_dir, old_sz, new_sz);
}

TK_STATUS vm_prog_load(vmm *vm, struct vm_prog_load_params *params)
{
    return x86_page_prog_load((struct x86_vmm *)vm, params);
}
