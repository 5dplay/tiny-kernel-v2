#include "common.h"
#include "vfs.h"
#include "limits.h"
#include "proc.h"
#include "string.h"

struct proc_file_ctx {
    struct proc *p;
    struct file *ofile[NOFILE];
    void *cwd;
    void *root;
} g_proc_file_ctx[NPROC];

int sys_setup()
{
    fs_init(ROOTDEV);

    memset(&g_proc_file_ctx, 0x0, sizeof(g_proc_file_ctx));
    return 0;
}

static struct proc_file_ctx* alloc_proc_ctx()
{
    int i;

    for (i = 0; i < NPROC; i++)
        if (g_proc_file_ctx[i].p != NULL)
            return &g_proc_file_ctx[i];

    return NULL;
}

static void free_proc_ctx()
{
    panic("TO BE DONE");
}

int proc_setup_fs(struct proc *p)
{
    int i;
    struct proc_file_ctx *ctx;

    for (i = 0; i < NPROC; i++) {
        if (g_proc_file_ctx[i].p == p) {
            printk("%s: %s already setup fs\n", __func__, p->comm);
            return 0;
        }
    }

    ctx = alloc_proc_ctx();

    return 0;
}
