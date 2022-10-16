#include "memlayout.h"
#include "mm.h"
#include "trap.h"
#include "type.h"
#include "string.h"
#include "common.h"
#include "proc.h"
#include "vm.h"

static struct proc s_proc_list[NPROC];
static int s_cur_proc = -1;
static int next_pid = 1;

void forkret()
{
    printk("%s: enter\n", __func__);
    return ;
}

static void prepare_usr_proc();

TK_STATUS proc_init()
{
    memset(s_proc_list, 0x0, sizeof(s_proc_list));
    prepare_usr_proc();
    return TK_STATUS_SUCCESS;
}

struct proc* get_cur_proc()
{
    if (0 <= s_cur_proc && s_cur_proc < NPROC)
        return &s_proc_list[s_cur_proc];
    else
        return NULL;
}

struct proc *proc_alloc()
{
    struct proc *p;
    int i;

    p = NULL;

    for (i = 0; i < NPROC; i++) {
        if (s_proc_list[i].state == UNUSED) {
            p = &s_proc_list[i];
            break;
        }
    }

    if (p == NULL)
        return NULL;

    p->state = EMBRYO;
    p->pid = next_pid++; /* FIXME: 这里最后会溢出，需要校验 */
    p->parent = NULL;
    memset(p->comm, 0x0, sizeof(p->comm));

    /* 分配内核栈，将pc指向fork_ret处 */
    arch_init_proc(p);

    return p;
}

void proc_free(struct proc *p)
{
    if (p == NULL)
        return ;

    p->state = UNUSED;
}

extern char _binary_user_init_code_start[], _binary_user_init_code_size[];

static void prepare_usr_proc()
{
    struct proc *usr_proc;
    void *pg;
    vmm *vm;

    usr_proc = proc_alloc();

    if (usr_proc == NULL)
        panic("failed to alloc proc for first proc!\n");

    vm = get_mmu();

    if (vm == NULL || (usr_proc->pg_dir = vm_alloc_pg_dir(vm)) == 0)
        panic("failed to init vm, vm = %p, pg_dir = %p\n", vm, usr_proc->pg_dir);

    kernel_map_init(vm, usr_proc->pg_dir);

    pg = page_alloc();

    if (pg == NULL)
        panic("out of memory for first proc!\n");

    usr_proc->mem_size = PAGE_SIZE;
    memcpy(pg, _binary_user_init_code_start, (uaddr)_binary_user_init_code_size);
    vm_map(vm, usr_proc->pg_dir, 0, phy_to_virt((uaddr)pg), PAGE_SIZE, VM_PERM_WRITE | VM_PERM_USER);

    strncpy_s(usr_proc->comm, "initcode", sizeof(usr_proc->comm));

    usr_init_proc(usr_proc);

    usr_proc->state = RUNNABLE;
}

void scheduler()
{
    struct proc *p;
    int i;

    s_cur_proc = -1;

    /* FIXME: 启动中断的函数为什么要放在循环体之外,曾经放置到循环体内部,导致中断出现异常.需要了解相关的知识. */
    enable_trap();

    while (1) {
        for (i = 0; i < NPROC; i++) {
            p = &s_proc_list[i];

            if (p->state != RUNNABLE)
                continue;

            s_cur_proc = i;
            switch_uvm(p);

            p->state = RUNNING;

            printk("%s join\n", p->comm);
            join(p);
            printk("reach here\n");
            switch_kvm();

            s_cur_proc = -1;
        }
    }
}
