#include "common.h"
#include "vfs.h"
#include "indexfs.h"
#include "limits.h"
#include "trap.h"
#include "proc.h"
#include "sys_call.h"
#include "string.h"

static int proc_add_file(struct file *f);
static struct file *proc_get_file(int fd);

//FIXME: 以下操作均无考虑日志操作, 后续需要思考在何处添加，以及如何添加的问题
int sys_open(void)
{
    char *path;
    int fd, mode;
    struct file *f;
    struct inode *ip;

    if (arg_ptr(0, (void **)&path) || arg_int(1, &mode)) {
        printk("%s: args err!\n", __func__);
        return -1;
    }

    /*
        1. 先寻找到文件路径对应的inode节点
        2. 申请新的文件描述符,并且将inode节点信息填入,更新当前进程的文件描述符表
    */

    if (mode & O_CREATE) {
        ip = do_createi(path, T_FILE, 0, 0);
    } else {
        ip = namei(path);
    }

    if (ip == NULL) {
        printk("%s: %s err\n", __func__, path);
        return -1;
    }

    f = file_alloc();

    if (f == NULL) {
        printk("%s: failed to alloc file desc.\n", __func__);
        goto free_inode;
    }

    fd = proc_add_file(f);

    if (fd < 0)
        goto free_file;

    f->type = FD_INODE;
    f->ip = ip;
    f->off = 0;
    f->flag = mode; //FIXME: O_CREATE呢?

    return fd;

free_file:
    //TODO: free_file

free_inode:
    //TODO: free_inode
    return -1;
}

//FIXME: 权限控制相关的?
int sys_read(void)
{
    struct file *f;
    int cnt, fd, ret;
    char *p;
    struct inode *ip;

    arg_int(0, &fd);
    arg_int(2, &cnt);
    arg_ptr(1, (void **)&p);

    f = proc_get_file(fd);

    if (f == NULL)
        return -1;

    ip = f->ip;

    if (ip->type == T_DEV) {
        if (ip->major > 0 && ip->major < NDEV && devsw[ip->major].read)
            return devsw[ip->major].read(ip, p, cnt);

        return -1;
    }

    ret = data_readi(ip->superb, ip, (u8 *)p, f->off, cnt);

    if (ret > 0) {
        f->off += ret;
        return ret;
    } else
        return -1;
}

int sys_write(void)
{
    struct file *f;
    int cnt, fd, ret;
    char *p;
    struct inode *ip;
    struct proc *pp;
    arg_int(0, &fd);
    arg_int(2, &cnt);
    arg_ptr(1, (void **)&p);


    pp = get_cur_proc();

    f = proc_get_file(fd);

    if (f == NULL)
        return -1;

    ip = f->ip;

    if (ip->type == T_DEV) {
        if (ip->major > 0 && ip->major < NDEV && devsw[ip->major].write)
            return devsw[ip->major].write(ip, p, cnt);

        return -1;
    }

    ret = data_writei(ip->superb, ip, (uint8_t *)p, f->off, cnt);

    if (ret > 0) {
        f->off += ret;
        return 0;
    } else
        return -1;
}

int sys_fseek(void)
{
    struct file *f;
    int fd, off, mode, tmp;

    arg_int(0, &fd);
    arg_int(1, &off);
    arg_int(2, &mode);

    f = proc_get_file(fd);

    switch (mode) {
        case SEEK_SET:
            tmp = off;
            break;

        case SEEK_CUR:
            tmp = f->off + off;
            break;

        case SEEK_END:
            tmp = f->ip->size + off;
            break;

        default:
            tmp = -1;
    }

    if (tmp < 0)
        return -1;

    f->off = tmp;
    return f->off;
}

int sys_unlink(void)
{
    struct inode *dp;
    char *path;
    char name[DNAME];

    arg_ptr(0, (void **)&path);
    dp = namei_parent(path, name);

    if (dp == NULL)
        return -1;

    if (strncmp(name, ".", sizeof(".") - 1) == 0 ||
        strncmp(name, "..", sizeof("..") - 1) == 0)
        goto free_dp;

    panic("not implement\n");
    return 0;
free_dp:
    //TODO: free dp
    return -1;
}

int sys_close(void)
{
    int fd;
    struct file *f;

    arg_int(0, &fd);

    f = proc_get_file(fd);

    if (f == NULL)
        return -1;

    panic("not implement\n");
    //TODO: close file desc
    return 0;
}

int sys_mknod(void)
{
    struct inode *ip;
    char *path;
    int major, minor;

    arg_ptr(0, (void **)&path);
    arg_int(1, &major);
    arg_int(2, &minor);

    ip = do_createi(path, T_DEV, major, minor);

    if (ip == NULL) {
        printk("failed to create dev node %s\n");
    }

    return 0;
}

int sys_dup(void)
{
    struct proc *cur;
    struct file *f;
    int i, fd;
    cur = get_cur_proc();

    arg_int(0, &fd);

    if (fd < 0 || fd >= NOFILE)
        return -1;

    f = cur->ofile[fd];

    for (i = 0; i < NOFILE; i++) {
        if (cur->ofile[i] == NULL) {
            cur->ofile[i] = f;
            f->ref++;
            return i;
        }
    }

    return -1;
}

int sys_fstat(void)
{
    struct proc *cur;
    struct file *f;
    struct stat *st;
    int fd;
    cur = get_cur_proc();

    arg_int(0, &fd);
    arg_ptr(1, (void **)&st);

    if (fd < 0 || fd >= NOFILE)
        return -1;

    f = cur->ofile[fd];

    if (f->type != FD_INODE)
        return -1;

    st->type = f->ip->type;
    st->n_link = f->ip->n_link;
    st->dev = f->ip->dev;
    st->ino = f->ip->idx;
    st->size = f->ip->size;

    return 0;
}

static int proc_add_file(struct file *f)
{
    struct proc *cur;
    int i;
    cur = get_cur_proc();

    for (i = 0; i < NOFILE; i++) {
        if (cur->ofile[i] == NULL) {
            cur->ofile[i] = f;
            return i;
        }
    }

    return -1;
}

static struct file *proc_get_file(int fd)
{
    struct proc *cur;

    if (fd < 0 || fd >= NOFILE)
        return NULL;

    cur = get_cur_proc();
    return cur->ofile[fd];
}
