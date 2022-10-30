#ifndef __TINY_KERNEL_TINYFS_H__
#define __TINY_KERNEL_TINYFS_H__

#include "type.h"
#include "indexfs.h"
#include "limits.h"

//内存超级块节点
struct tinyfs_superblock {
    struct superblock parent;

    u32 size;
    u32 n_blocks;
    u32 n_inodes;
    u32 inode_off;
    u32 bitmap_off;
};

//磁盘中超级块信息, 需要自己转字节序
struct tinyfs_superblock_disk {
#define TINY_FS_MAGIC 0x1996
    u32 magic;
    u32 size;
    u32 n_blocks;
    u32 n_inodes;
    u32 inode_off;
    u32 bitmap_off;
};

#define BSIZE 512
#define NDIRECT 12
#define NINDIRECT (BSIZE/sizeof(u32))
#define MAXFILE (NDIRECT+NINDIRECT)

struct tinyfs_inode_disk {
    u16 type;                  //类型
    u16 major;                 //主设备号
    u16 minor;                 //次设备号
    u16 n_link;                //链接个数
    u32 size;                  //文件大小
    u32 addr[NDIRECT + 1];     //数据块地址, NDIRECT个直接寻址外加一个间接寻址
};

//inode per block 一个数据块最大容纳inode节点的个数
#define IPB (BSIZE/sizeof(struct tinyfs_inode_disk))

//获取inode i对应所在的的数据块序号
#define IBLOCK(i, sb) ((i)/IPB + (sb)->inode_off)

//一个数据块最大容纳的的位个数
#define BPB (BSIZE*8)

//获取bit b对应所在的的数据块序号
#define BBLOCK(b, sb) ((b)/BPB + (sb)->bitmap_off)

struct tinyfs_dirent_disk {
    u16 idx;
    char name[DNAME];
};

struct tinyfs_inode {
    struct inode parent;

    u32 addr[NDIRECT + 1];
};

#endif
