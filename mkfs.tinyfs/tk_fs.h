#ifndef __TINY_KERNEL_FS_H__
#define __TINY_KERNEL_FS_H__

#include <stdint.h>
//copy from xv6-public, big endian

#define FS_BLOCK_SIZE 512
#define FS_ROOT_INODE 1
#define FS_INODE_L0_MAX_NUM 12
#define FS_INODE_BLOCK_L1 FS_INODE_L0_MAX_NUM
#define FS_INODE_L1_MAX_NUM 1
#define FS_INODE_LN_MAX_NUM (FS_INODE_L0_MAX_NUM + FS_INODE_L1_MAX_NUM)
#define FS_FILE_NAME_SIZE 14
#define FS_L0_FILE_MAX_NUM FS_INODE_L0_MAX_NUM
#define FS_L1_FILE_MAX_NUM (FS_INODE_L1_MAX_NUM*(FS_BLOCK_SIZE/sizeof(uint32_t)))

#define FS_FILE_MAX_NUM (FS_L0_FILE_MAX_NUM+FS_L1_FILE_MAX_NUM)
#define FS_MAX_OP_BLOCKS_NUM 10
#define FS_MAX_LOG_BLOCKS_NUM (FS_MAX_OP_BLOCKS_NUM*3)

//on-disk super block structure, 内存中-主机字节序，磁盘中-网络字节序
struct tk_superblock {
    uint32_t magic;
    uint32_t size;              //size of file system in blocks
    uint32_t n_blocks;          //number of data blocks
    uint32_t n_inodes;          //number of inodes
    uint32_t inodes_off;        //block number of first inode block
    uint32_t bitmap_off;        //blocl number of first bitmap block
} __attribute__((packed));

//on-disk inode structure, 内存中-主机字节序，磁盘中-网络字节序
struct tk_inode {
    uint16_t type;                              //inode type
    uint16_t major;
    uint16_t minor;
    uint16_t n_links;                           //number of links to inode in file system
    uint32_t file_size;                         //size of file in bytes
    uint32_t blocks[FS_INODE_LN_MAX_NUM];
} __attribute__((packed));

#define FS_INODE_NUM_PER_BLOCK (FS_BLOCK_SIZE/(sizeof(struct tk_inode)))

#define FS_BITS_PER_BLOCK (FS_BLOCK_SIZE*8)

static inline uint32_t get_inode_block(struct tk_superblock *sb, uint32_t i_no)
{
    return sb->inodes_off + i_no / FS_INODE_NUM_PER_BLOCK;
}

static inline uint32_t get_bitmap_block(struct tk_superblock *sb, uint32_t bit)
{
    return sb->bitmap_off + bit / FS_BITS_PER_BLOCK;
}

struct tk_dirent {
    uint16_t i_no;                  //inode number
    char name[FS_FILE_NAME_SIZE];   //directory entry name
} __attribute__((packed));

enum TK_STAT_E {
    TK_STAT_DIR = 1,        //directory
    TK_STAT_FILE = 2,       //file
    TK_STAT_DEV = 3,        //device
};

#endif
