#ifndef __TINY_KERNEL_BLOCK_DEV_H__
#define __TINY_KERNEL_BLOCK_DEV_H__

#include "type.h"


struct block_buf;
struct block_dev {
    u32 dev;                       //设备号
    u32 valid;
    void (*rw)(struct block_buf *);     //块设备读写操作
};

struct block_dev* get_block_dev(u32 dev);
int register_block_dev(u32 dev, void (*rw)(struct block_buf *));
int block_dev_init();

#endif
