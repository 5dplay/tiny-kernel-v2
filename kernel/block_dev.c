#include "common.h"
#include "block_dev.h"
#include "limits.h"

struct block_dev g_block_dev[NDEV];

int register_block_dev(u32 dev, void (*rw)(struct block_buf *))
{
    int i, slot;

    for (i = 0, slot = -1; i < NDEV; i++) {
        if (g_block_dev[i].dev == dev && g_block_dev[i].valid)
            return 0;

        if (slot < 0 && !g_block_dev[i].valid)
            slot = i;
    }

    if (slot < 0) {
        printk("too many devs.\n");
        return -1;
    }

    g_block_dev[slot].dev = dev;
    g_block_dev[slot].rw = rw;
    g_block_dev[slot].valid = 1;

    return 0;
}

struct block_dev* get_block_dev(u32 dev)
{
    int i;

    for (i = 0; i < NDEV; i++) {
        if (g_block_dev[i].dev == dev && g_block_dev[i].valid)
            return &g_block_dev[i];
    }

    return NULL;
}
