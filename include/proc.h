#ifndef __TINY_KERNEL_PROC_H__
#define __TINY_KERNEL_PROC_H__

#include "type.h"
#include "limits.h"
enum proc_state {UNUSED, EMBRYO, SLEEPING, RUNNABLE, RUNNING, ZOMBIE};

#define PROC_NAME_MAX_SIZE 16
#define NPROC 64


struct file;
struct inode;
struct proc {
    enum proc_state state;              /* 当前进程的状态 */

    char comm[PROC_NAME_MAX_SIZE];      /* 进程名 */
    int pid;                            /* 进程id */
    struct proc *parent;                /* 父进程id */

    void *wait_obj;

    /* fs start */
    struct file *ofile[NOFILE];
    struct inode *cwd;
    struct inode *root;
    /* fs end */

    uaddr pg_dir;                       /* 虚拟内存相关 */
    u32 mem_size;

    u8 arch_proc[128];                  /* 各个arch独自定义结构体，这里只是简单处理 */
};

/**
* @brief 初始化
*
* @return
*/
TK_STATUS proc_init();


/* 各arch独自定义 -- start */
void arch_init_proc(struct proc *p);
void arch_free_proc(struct proc *p);

/* 首个用户进程各arch最终初始化 */
void usr_init_proc(struct proc *p);

/* for do_exec */
void usr_exec_proc(struct proc *p, uaddr ep, u32 sp);

/* for do_fork */
int trap_fork_proc(struct proc *dst, struct proc *src);

void join(struct proc *p);

void yield();
/* 各arch独自定义 -- end */

/* TODO: 单独挪到sched.h */
void scheduler();

/**
* @brief 分配一个新的进程描述符
*
* @return 失败 == NULL
*/
struct proc* proc_alloc();

/**
* @brief 释放进程描述符
*
* @param [in] p 进程描述符
*/
void proc_free(struct proc *p);

/**
* @brief 获取当前运行的进程
*
* @return 失败 == NULL
*/
struct proc* get_cur_proc();

void sleep(void *obj);

void wakeup(void *obj);

#endif
