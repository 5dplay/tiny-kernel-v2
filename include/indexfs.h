#ifndef __TINY_KERNEL_INDEXFS_H__
#define __TINY_KERNEL_INDEXFS_H__

#include "type.h"

#define T_DIR 1
#define T_FILE 2
#define T_DEV 3

#define ROOTINO 1
#define DNAME 14

struct inode_op;
struct superblock;
struct inode {
    u32 dev;
    u16 idx;
    u16 type;
    u16 major;
    u16 minor;
    u16 n_link;
    u16 ref;
    u16 sync;
    u32 size;

    struct superblock *superb;
};

struct superblock_op;
struct superblock {
    u32 dev;
    struct inode *root;
    struct superblock_op *op;
};

struct superblock_op {
    struct inode* (*geti)(struct superblock *sb, u32 idx);
    struct inode* (*alloci)(struct superblock *sb, u16 type, u16 major, u16 minor);
    int (*meta_readi)(struct superblock *sb, struct inode *ip);
    int (*meta_writei)(struct superblock *sb, struct inode *ip);
    struct inode* (*lookupi)(struct superblock *sb, struct inode *src, const char *dname, int size);
    int (*data_readi)(struct superblock *sb, struct inode *i, u8 *dst, int off, int cnt);
    int (*data_writei)(struct superblock *sb, struct inode *i, u8 *src, int off, int cnt);
    int (*data_linki)(struct superblock *sb, struct inode *dst, struct inode *src, const char *dname, int size);
};

static inline struct inode* geti(struct superblock *sb, u32 idx)
{
    return sb->op->geti(sb, idx);
}

static inline struct inode* alloci(struct superblock *sb, u16 type, u16 major, u16 minor)
{
    return sb->op->alloci(sb, type, major, minor);
}

static inline int meta_readi(struct superblock *sb, struct inode *ip)
{
    return sb->op->meta_readi(sb, ip);
}

static inline int meta_writei(struct superblock *sb, struct inode *ip)
{
    return sb->op->meta_writei(sb, ip);
}

static inline struct inode* lookupi(struct superblock *sb, struct inode *src, const char *dname, int size)
{
    return sb->op->lookupi(sb, src, dname, size);
}

static inline int data_readi(struct superblock *sb, struct inode *i, u8 *dst, int off, int cnt)
{
    return sb->op->data_readi(sb, i, dst, off, cnt);
}

static inline int data_writei(struct superblock *sb, struct inode *i, u8 *src, int off, int cnt)
{
    return sb->op->data_writei(sb, i, src, off, cnt);
}

static inline int data_linki(struct superblock *sb, struct inode *dst, struct inode *src, const char *dname, int size)
{
    return sb->op->data_linki(sb, dst, src, dname, size);
}

struct superblock* fs_init();
struct inode* namei(char *path);
void dupi(struct inode *ip);
struct inode* namei_parent(char *path, char *name);
struct inode* do_createi(char *path, u16 type, u16 major, u16 minor);

#endif
