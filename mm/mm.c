#include "mm.h"
#include "vm.h"
#include "common.h"
#include "type.h"
#include "memlayout.h"

int g_mm_initialized = 0;
uaddr kernel_pg_dir;
vmm *vm;

struct mmap_desc {
    uaddr virt;
    uaddr phys_start;
    u32 phys_end;
    u32 flags;
};

//FIXME: 其实按照xv6的实现,kernel code和rodata部分应该为只读
static struct mmap_desc kmmap[] = {
    {KERNEL_BASE, 0, PHY_8M, VM_PERM_WRITE}, /*可读可写*/
    {KERNEL_BASE + PHY_8M, PHY_8M, PHY_TOP, VM_PERM_WRITE}, /*可读可写*/
};
#define kmmap_size (sizeof(kmmap)/sizeof(kmmap[0]))

//FIXME: 这两个在完成重构后升级成全局函数
TK_STATUS kernel_map_init(vmm *p_vm, uaddr pg_dir)
{
    int i;

    //初始化kernel部分虚拟内存映射
    for (i = 0; i < kmmap_size; i++) {
        vm_map(p_vm, pg_dir, kmmap[i].virt, kmmap[i].phys_start,
               kmmap[i].phys_end - kmmap[i].phys_start, kmmap[i].flags);
    }

#ifdef ARCH_KERNEL_MAP_INIT
    extern int arch_kernel_map_init(vmm * p_vm, uaddr pg_dir);
    arch_kernel_map_init(p_vm, pg_dir);
#endif

    return TK_STATUS_SUCCESS;
}

TK_STATUS mm_init()
{
    TK_STATUS ret;

    ret = TK_STATUS_FAILURE;
    kernel_pg_dir = 0;

    vm = arch_mmu_init();

    if (vm == NULL) {
        early_print("failed to arch vm init!\n");
        return ret;
    }

    kernel_pg_dir = vm_alloc_pg_dir(vm);

    if (kernel_pg_dir == 0x0) {
        early_print("failed to alloc pg_dir\n");
        return ret;
    }

    ret = kernel_map_init(vm, kernel_pg_dir);

    if (TK_STATUS_IS_ERR(ret)) {
        early_print("failed to kernel map init\n");
        vm_free_pg_dir(vm, kernel_pg_dir);
        return ret;
    }

    vm_reload(vm, kernel_pg_dir);

    return TK_STATUS_SUCCESS;
}

vmm *get_mmu()
{
    return vm;
}

TK_STATUS kmap(uaddr v_addr, uaddr p_addr, uint size, uint perm_flags)
{
    return vm_map(vm, kernel_pg_dir, v_addr, p_addr, size, perm_flags);
}

void switch_kvm()
{
    vm_reload(vm, kernel_pg_dir);
}
