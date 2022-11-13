#include "common.h"
#include "memlayout.h"
#include "mm.h"
#include "vm.h"
#include "string.h"
#include "proc.h"
#include "arm_asm.h"
#include "arm_proc.h"
#include "arm_trap.h"

extern void arm_forkret_entry(void);
extern void trapret(void);
extern char arm_vector_start[], arm_vector_end[];
#define ARM_VECTOR_BASE 0xFFFF0000

#if 0
void *get_cur_proc_kstack_top()
{
    struct proc *p;
    struct arm_proc *a;

    p = get_cur_proc();
    a = (struct arm_proc *)p->arch_proc;
    return a->kstack + PAGE_SIZE;
}
#endif

void arch_init_proc(struct proc *p)
{
    u8 *sp;
    struct arm_proc *a;

    if (p == NULL || sizeof(*a) > sizeof(p->arch_proc))
        panic("p = %p, sizeof(arm_proc) = %d\n", p, sizeof(*a));

    a = (struct arm_proc *)p->arch_proc;

    a->kstack = page_alloc();

    sp = a->kstack + PAGE_SIZE;

    /* trap frame */
    sp -= sizeof(*a->tf);
    a->tf = (struct trap_frame *)sp;
    /* push {lr} ; lr == trapret */
    sp -= sizeof(uaddr);
    *(uaddr *)sp = (uaddr)trapret;

    sp -= sizeof(*a->ctx);
    a->ctx = (struct arm_context *)sp;
    memset(a->ctx, 0x0, sizeof(*a->ctx));
    a->ctx->lr = (uaddr)arm_forkret_entry;
}

int arch_kernel_map_init(vmm *p_vm, uaddr pg_dir)
{
    vm_map(p_vm, pg_dir, KERNEL_BASE + MMIO_BASE_PHY, MMIO_BASE_PHY, PHY_4M, VM_PERM_WRITE);
    vm_map(p_vm, pg_dir, ARM_VECTOR_BASE, virt_to_phy((uaddr)arm_vector_start), arm_vector_end - arm_vector_start, VM_PERM_WRITE);
    return 0;
}

void usr_init_proc(struct proc *p)
{
    struct arm_proc *a;
    u32 cpsr;

    if (p == NULL || sizeof(*a) > sizeof(p->arch_proc))
        panic("p = %p, sizeof(arm_proc) = %d\n", p, sizeof(*a));

    a = (struct arm_proc *)p->arch_proc;

    memset(a->tf, 0x0, sizeof(*a->tf));

    get_cpsr(&cpsr);
    cpsr &= ~I_BIT;
    cpsr &= ~MODE_MASK;
    cpsr |= MODE_USER;
    a->tf->cpsr = cpsr;
    a->tf->sp = PAGE_SIZE;
    a->tf->lr = 0x0;
}

extern void switch_ctx(struct arm_context **old_ctx, struct arm_context *new_ctx);
struct arm_context *g_arm_ctx_sched;

void join(struct proc *p)
{
    struct arm_proc *a;

    if (p == NULL || sizeof(*a) > sizeof(p->arch_proc))
        panic("p = %p, sizeof(arm_proc) = %d\n", p, sizeof(*a));

    a = (struct arm_proc *)p->arch_proc;
    switch_ctx(&g_arm_ctx_sched, a->ctx);
}

void yield()
{
    struct proc *p;
    struct arm_proc *a;

    p = get_cur_proc();

    if (p == NULL || sizeof(*a) > sizeof(p->arch_proc))
        panic("p = %p, sizeof(arm_proc) = %d\n", p, sizeof(*a));

    a = (struct arm_proc *)p->arch_proc;
    switch_ctx(&a->ctx, g_arm_ctx_sched);
}

void switch_uvm(struct proc *p)
{
    struct arm_proc *a;

    if (p == NULL || sizeof(*a) > sizeof(p->arch_proc))
        panic("p = %p, sizeof(arm_proc) = %d\n", p, sizeof(*a));

    a = (struct arm_proc *)p->arch_proc;
    vm_reload(get_mmu(), p->pg_dir);
}

void usr_exec_proc(struct proc *p, uaddr ep, u32 sp)
{
    struct arm_proc *a;

    if (p == NULL || sizeof(*a) > sizeof(p->arch_proc))
        panic("p = %p, sizeof(arm_proc) = %d\n", p, sizeof(*a));

    a = (struct arm_proc *)p->arch_proc;
    a->tf->pc = ep;
    a->tf->sp = sp;
}

void arch_free_proc(struct proc *p)
{
    struct arm_proc *x;

    if (p == NULL || sizeof(struct arm_proc) > sizeof(p->arch_proc))
        panic("p = %p, sizeof(arm_proc) = %d\n", p, sizeof(struct arm_proc));

    x = (struct arm_proc *)p->arch_proc;
    page_free(x->kstack);
}

int trap_fork_proc(struct proc *dst, struct proc *src)
{
    struct arm_proc *dx, *sx;

    dx = (struct arm_proc *)dst->arch_proc;
    sx = (struct arm_proc *)src->arch_proc;
    memcpy(dx->tf, sx->tf, sizeof(*dx->tf));
    //fork出来的子进程返回0
    dx->tf->r0 = 0;
    return 0;
}
