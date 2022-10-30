#include "common.h"
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
    /* push {fp, lr} ; lr == trapret */
    sp -= sizeof(uaddr);
    *(uaddr *)sp = (uaddr)trapret;
    sp -= sizeof(uaddr);
    *(uaddr *)sp = (uaddr)(a->tf);

    sp -= sizeof(*a->ctx);
    a->ctx = (struct arm_context *)sp;
    memset(a->ctx, 0x0, sizeof(*a->ctx));
    a->ctx->lr = (uaddr)arm_forkret_entry;
}

void usr_init_proc(struct proc *p)
{
    struct arm_proc *a;
    u32 cpsr;

    if (p == NULL || sizeof(*a) > sizeof(p->arch_proc))
        panic("p = %p, sizeof(arm_proc) = %d\n", p, sizeof(*a));

    a = (struct arm_proc *)p->arch_proc;
    vm_map(get_mmu(), p->pg_dir, ARM_VECTOR_BASE, virt_to_phy((uaddr)arm_vector_start), arm_vector_end - arm_vector_start, VM_PERM_WRITE);

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
    printk("join finish\n");
}

void yield(struct proc *p)
{
    panic("%s: TO BE DONE!\n", __func__);
}

void switch_uvm(struct proc *p)
{
    struct arm_proc *a;

    if (p == NULL || sizeof(*a) > sizeof(p->arch_proc))
        panic("p = %p, sizeof(arm_proc) = %d\n", p, sizeof(*a));

    a = (struct arm_proc *)p->arch_proc;
    vm_reload(get_mmu(), p->pg_dir);
}