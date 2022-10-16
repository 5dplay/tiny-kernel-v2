#include "common.h"
#include "proc.h"
#include "x86_proc.h"
#include "mm.h"
#include "vm.h"
#include "string.h"
#include "x86_seg.h"


extern void trapret();
extern void forkret();

void arch_init_proc(struct proc *p)
{
    u8 *sp;
    struct x86_proc *x;

    if (p == NULL || sizeof(struct x86_proc) > sizeof(p->arch_proc))
        panic("p = %p, sizeof(x86_proc) = %d\n", p, sizeof(struct x86_proc));

    x = (struct x86_proc *)p->arch_proc;

    x->kstack = page_alloc();

    sp = x->kstack + PAGE_SIZE;

    /* trap frame */
    sp -= sizeof(*x->tf);
    x->tf = (struct trap_frame *)sp;

    /* 返回地址是trapret */
    sp -= 4;
    *(uaddr *)sp = (uaddr)trapret;

    /* 将当前pc指向forkret */
    sp -= sizeof(*x->ctx);
    x->ctx = (struct x86_context *)sp;
    memset(x->ctx, 0x0, sizeof(*x->ctx));
    x->ctx->eip = (uaddr)forkret;
}

void usr_init_proc(struct proc *p)
{
    struct x86_proc *x;

    if (p == NULL || sizeof(struct x86_proc) > sizeof(p->arch_proc))
        panic("p = %p, sizeof(x86_proc) = %d\n", p, sizeof(struct x86_proc));

    x = (struct x86_proc *)p->arch_proc;

    memset(x->tf, 0x0, sizeof(*x->tf));
    x->tf->cs = (SEG_UCODE << 3) | 0x3;
    x->tf->ds = (SEG_UDATA << 3) | 0x3;
    x->tf->es = x->tf->ds;
    x->tf->ss = x->tf->ds;

    x->tf->eflags = 0x200; // intr enable
    x->tf->esp = PAGE_SIZE;
    x->tf->eip = 0; //beginning of initcode.S
}

/**
* @brief 存储旧的上下文信息,并且加载新的上下文信息
*
* @param [out]old_ctx 存储旧的上下文信息
* @param [in]new_ctx 新的上下文信息
*/
void switch_ctx(struct x86_context **old_ctx, struct x86_context *new_ctx);

struct x86_context *g_x86_ctx_sched;

void join(struct proc *p)
{
    struct x86_proc *x;

    if (p == NULL || sizeof(struct x86_proc) > sizeof(p->arch_proc))
        panic("p = %p, sizeof(x86_proc) = %d\n", p, sizeof(struct x86_proc));

    x = (struct x86_proc *)p->arch_proc;

    switch_ctx(&g_x86_ctx_sched, x->ctx);
}

void yield()
{
    struct proc *p;
    struct x86_proc *x;

    p = get_cur_proc();

    if (p == NULL || sizeof(struct x86_proc) > sizeof(p->arch_proc))
        panic("p = %p, sizeof(x86_proc) = %d\n", p, sizeof(struct x86_proc));

    x = (struct x86_proc *)p->arch_proc;
    switch_ctx(&x->ctx, g_x86_ctx_sched);
}

void switch_uvm(struct proc *p)
{
    struct x86_proc *x;

    if (p == NULL || sizeof(struct x86_proc) > sizeof(p->arch_proc))
        panic("p = %p, sizeof(x86_proc) = %d\n", p, sizeof(struct x86_proc));

    x = (struct x86_proc *)p->arch_proc;
    setup_tss((struct x86_vmm *)get_mmu(), (uaddr)(x->kstack + PAGE_SIZE));
    vm_reload(get_mmu(), p->pg_dir);
}
