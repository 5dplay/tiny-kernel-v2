/*
 * FIXME: 本来打算构建一层抽象的文件系统，但是感觉设计过度了，还是采用原设计
 */

#include "common.h"
#include "vfs.h"
#include "indexfs.h"
#include "limits.h"
#include "proc.h"
#include "string.h"

int sys_setup()
{
    struct superblock *superb;
    struct proc *p;
    superb = fs_init(ROOTDEV);

    p = get_cur_proc();
    p->cwd = p->root = superb->root;
    return 0;
}

struct file file_cached[NFILE];
/* struct devsw devsw[NDEV]; */

struct file* file_alloc()
{
    int i;

    for (i = 0; i < NFILE; i++) {
        if (file_cached[i].ref == 0) {
            file_cached[i].ref = 1;
            return &file_cached[i];
        }
    }

    return NULL;
}

struct file* file_dup(struct file *f)
{
    f->ref++;
    return f;
}

int file_close(struct file *f)
{
    f->ref--;
    //FIXME: so many sth to do
    return 0;
}
