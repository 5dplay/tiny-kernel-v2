#ifndef __TINY_KERNEL_VFS_H__
#define __TINY_KERNEL_VFS_H__

#include "type.h"
#include "limits.h"

#define O_RDONLY  0x000
#define O_WRONLY  0x001
#define O_RDWR    0x002
#define O_CREATE  0x200

struct file_operations;
struct inode;
struct file {
    enum {FD_NONE, FD_INODE, FD_REG} type;
    int ref;
    int flag;

    struct inode *ip;
    u32 off;

    struct file_operations *f_op;
};

struct file_operations {
    int (*fread)(struct file* f, void *dst, int cnt);
    int (*fwrite)(struct file* f, void *src, int cnt);
    int (*fseek)(struct file *f, uint32_t off, int where);
    int (*fclose)(struct file *f);
};

static inline int fread(struct file* f, void *dst, int cnt)
{
    if (f->f_op->fread)
        return f->f_op->fread(f, dst, cnt);
    else
        return -1;
}

static inline int fwrite(struct file* f, void *src, int cnt)
{
    if (f->f_op->fwrite)
        return f->f_op->fwrite(f, src, cnt);
    else
        return -1;
}

static inline int fseek(struct file *f, uint32_t off, int where)
{
    if (f->f_op->fseek)
        return f->f_op->fseek(f, off, where);
    else
        return -1;
}

static inline int fclose(struct file *f)
{
    if (f->f_op->fclose)
        return f->f_op->fclose(f);
    else
        return -1;
}

struct file* file_alloc(void);
struct file* file_dup(struct file *);
int file_close(struct file *f);

// table mapping major device number to
// device functions
struct devsw {
    int (*read)(struct inode*, char*, int);
    int (*write)(struct inode*, char*, int);
};

extern struct devsw devsw[];
#define CONSOLE 1
int console_init();
void console_intr();
void console_putc(int c);
int console_getc();

struct stat {
    u16 type;
    u16 n_link;
    u32 dev;
    u32 ino;
    u32 size;
};

enum {
    SEEK_SET = 0,       //设置新的读写位置
    SEEK_CUR,           //在当前读写位置
    SEEK_END            //在文件尾部位置
};

#endif
