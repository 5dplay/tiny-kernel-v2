#include "memlayout.h"
#include "mm.h"
#include "trap.h"
#include "type.h"
#include "string.h"
#include "common.h"
#include "proc.h"
#include "vm.h"
#include "vfs.h"
#include "indexfs.h"

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

#if 0
            printk("%s join\n", p->comm);
#endif
            join(p);
            switch_kvm();

            s_cur_proc = -1;
        }
    }
}

int sys_fork(void)
{
    int sub_pid, i;
    struct proc *sub_proc, *cur;
    vmm *vm;

    //分配一个新的进程描述符
    cur = get_cur_proc();
    sub_proc = proc_alloc();

    if (sub_proc == NULL) {
        printk("failed to alloc proc\n");
        return -1;
    }

    vm = get_mmu();
    sub_proc->parent = cur;
    //拷贝当前进程的内存
    sub_proc->pg_dir = vm_alloc_pg_dir(vm);

    kernel_map_init(vm, sub_proc->pg_dir);

    //拷贝用户进程的内存映射
    if (vm_clone_pg_dir(vm, sub_proc->pg_dir, cur->pg_dir, cur->mem_size) < 0) {
        printk("failed to fork proc\n");
        goto free_proc;
    }

    sub_proc->mem_size = cur->mem_size;

    //拷贝当前进程的文件描述符
    for (i = 0; i < NOFILE; i++)
        if (cur->ofile[i])
            sub_proc->ofile[i] = file_dup(cur->ofile[i]);

    //拷贝特定的代码段相关
    trap_fork_proc(sub_proc, cur);

    sub_proc->cwd = cur->cwd;
    sub_proc->root = cur->root;
    dupi(cur->cwd);
    dupi(cur->root);

    strncpy(sub_proc->comm, cur->comm, sizeof(sub_proc->comm) - 1);
    sub_pid = sub_proc->pid;
    sub_proc->state = RUNNABLE;
    //返回子进程的pid
    return sub_pid;

free_proc:
    //FIXME: too many exception handle
    return -1;
}

int sys_exit(void)
{
    struct proc *cur;
    int fd;
    cur = get_cur_proc();

    for (fd = 0; fd < NOFILE; fd++) {
        if (cur->ofile[fd]) {
            file_close(cur->ofile[fd]);
            cur->ofile[fd] = NULL;
        }
    }

    cur->cwd->ref--;
    cur->cwd = NULL;
    cur->root->ref--;
    cur->root = NULL;

    //FIXME: 需要将reparent所有的child

    cur->state = ZOMBIE;
    wakeup(cur->parent);
    yield();
    panic("zombie never reach\n");
    return 0;
}

int sys_wait(void)
{
    struct proc *cur, *p;
    int pid, cnt, i;
    cur = get_cur_proc();

    while (1) {
        cnt = 0;

        for (i = 0; i < NPROC; i++) {
            p = &s_proc_list[i];

            if (p->parent != cur)
                continue;

            cnt++;

            if (p->state == ZOMBIE) {
                arch_free_proc(p);
                pid = p->pid;
                //FIXME: 这里会有内存泄露？
                vm_free_pg_dir(get_mmu(), p->pg_dir);
                p->pid = 0;
                p->parent = NULL;
                p->comm[0] = '\0';
                p->state = UNUSED;
                return pid;
            }
        }

        if (!cnt)
            return -1;

        sleep(cur);
    }

    return -1;
}
void sleep(void *obj)
{
    struct proc *p;

    p = get_cur_proc();

    disable_trap();
    p->wait_obj = obj;
    p->state = SLEEPING;
    enable_trap();
    yield();

    p->wait_obj = NULL;
}

void wakeup(void *obj)
{
    struct proc *p;
    int i;

    p = NULL;

    for (i = 0; i < NPROC; i++) {
        p = &s_proc_list[i];

        if (p->state == SLEEPING && p->wait_obj == obj) {
            p->state = RUNNABLE;
        }

    }
}
