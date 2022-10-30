#include "common.h"
#include "sys_call.h"
#include "proc.h"

// number of elements in fixed-size array
#define NELEM(x) (sizeof(x)/sizeof((x)[0]))

#if 0
extern int sys_fork(void);
extern int sys_wait(void);
extern int sys_exit(void);
extern int sys_exec(void);
extern int sys_dup(void);
extern int sys_open(void);
extern int sys_close(void);
extern int sys_read(void);
extern int sys_write(void);
extern int sys_mknod(void);
extern int sys_fstat(void);
extern int sys_sleep(void);
#endif

extern int sys_setup(void);
static int (*sys_calls[])(void) = {
#if 0
    [SYS_dup] = sys_dup,
    [SYS_open] = sys_open,
    [SYS_close] = sys_close,
    [SYS_read] = sys_read,
    [SYS_write] = sys_write,
    [SYS_mknod] = sys_mknod,
    [SYS_fstat] = sys_fstat,
    [SYS_fork] = sys_fork,
    [SYS_wait] = sys_wait,
    [SYS_exit] = sys_exit,
    [SYS_exec] = sys_exec,
    [SYS_test] = sys_test,
    [SYS_sleep] = sys_sleep,
#endif
    [SYS_setup] = sys_setup,
};

int sys_call(int idx)
{
    struct proc *p = get_cur_proc();

    if (idx > 0 && idx < NELEM(sys_calls) && sys_calls[idx])
        return sys_calls[idx]();
    else {
        printk("pid:%d, comm:%s: unknown sys call %d\n",
               p->pid, p->comm, idx);
        return -1;
    }
}
