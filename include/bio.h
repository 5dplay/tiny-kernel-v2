#ifndef __TINY_KERNEL_BIO_H__
#define __TINY_KERNEL_BIO_H__

#include "type.h"
#include "list.h"

#define BLOCK_BUF_NUM    30
#define BLOCK_BUF_SIZE   512
#define BLOCK_FLAG_VALID 0x2         //缓冲区数据已于硬盘数据同步
#define BLOCK_FLAG_DIRTY 0x4         //已修改,但是缓冲区数据尚未写入硬盘数据中,只有写操作才会打上这个标记

struct block_buf {
    volatile u32 flags;    //avoid gcc compiler optimization : while(CHECK_FLAG(buf->flag)) ;
    u32 dev;
    u32 idx;
    u32 ref_cnt;
    struct list_head queue;
    uint8_t data[BLOCK_BUF_SIZE];
};

#define BLOCK_IS_VALID(buf) (((buf)->flags)&BLOCK_FLAG_VALID)
#define BLOCK_IS_DIRTY(buf) (((buf)->flags)&BLOCK_FLAG_DIRTY)


struct block_dev;
/**
* @brief read from ide
*
* @param dev
* @param block_idx
*
* @return
*/
struct block_buf* bio_read(struct block_dev *dev, u32 block_idx);

/**
* @brief flush dirty buf to disk
*
* @param buf
*/
void bio_write(struct block_dev *dev, struct block_buf *buf);

/**
* @brief release block buffer
*
* @param buf
*/
void bio_free(struct block_buf *buf);

int bio_init(void);

/**
* @brief 申请下一次块设备读写情求
*/
void bio_sched(struct block_dev *dev, struct block_buf *buf);
/**
* @brief 获取当前等待读写的buf, 并且从队列中弹出
*
* @return 缓冲区
*/
struct block_buf* bio_pop();

/**
* @brief 当前队列第一个元素
*
* @return 缓冲区
*/
struct block_buf* bio_front();
#endif
