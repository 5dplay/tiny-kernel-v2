#ifndef __FS_INODE_OP_H__
#define __FS_INODE_OP_H__

#include <stdint.h>
#include "tk_fs.h"

struct inode_info {
    uint32_t idx;           //inode对应的序号，序列化时候使用
    struct tk_inode info;   //inode信息，主机字节序
};

struct superblock_info {
    uint32_t inode_free;        //当前未使用的inode序号，默认从1开始
    uint32_t block_free;        //当前未使用的块号，需要计算偏移得到
    int fd;                     //文件句柄
    struct tk_superblock *sb;   //超级块信息，主机字节序
};

/**
* @brief 获取序号对应的inode信息（已转成主机字节序）
*
* @param [in]sb_info 超级块信息（主机字节序）
* @param [in]inode 序号
* @param [out]i_info inode
*
* @return 0 成功，-1 失败
*/
int get_inode_info(struct superblock_info *sb_info, uint32_t inode,
                   struct inode_info *i_info);

/**
* @brief 序列化inode信息到磁盘中
*
* @param [in]sb_info 超级块信息（主机字节序）
* @param [in]i_info inodex信息
*
* @return 0 成功， -1 失败
*/
int put_inode_info(struct superblock_info *sb_info, const struct inode_info *i_info);

/**
* @brief 在inode节点数据追加数据, 注意此函数并不会将inode节点更新，仅更新data相关的
*
* @param [in]sb_info 超级快信息
* @param [in]fd 文件句柄
* @param [in]inode inode
* @param [in]data 数据
* @param [in]size
*
* @return 0 成功， -1 失败
*/
int inode_append(struct superblock_info *sb_info, struct inode_info *inode,
                 const void *data, uint32_t size);

/**
* @brief 分配一个inode节点
*
* @param [in]sb_info 超级块信息
* @param [in]fd 文件句柄
* @param [in]type 类型
* @param [out]i_info inode
*
* @return 0 成功， -1 失败
*/
int inode_alloc(struct superblock_info *sb_info, uint16_t type,
                struct inode_info *i_info);

/**
* @brief 分配一个块
*
* @param [in]sb_info 超级块
*
* @return 块号
*/
uint32_t block_alloc(struct superblock_info *sb_info);

/**
* @brief 标记已使用block [0, end)
*
* @param [in]sb_info 超级块
* @param [in]end 结束块号 end >= 1
*
*/
void block_mark_alloc(struct superblock_info *sb_info, uint32_t end);

void inode_m2d(struct tk_inode *info);
void inode_d2m(struct tk_inode *info);
void superblock_m2d(struct tk_superblock *sb);
void superblock_d2m(struct tk_superblock *sb);

#endif
