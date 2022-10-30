#include "common.h"
#include "block_dev.h"
#include "bio.h"
#include "list.h"
//#include "sched.h"
#include "string.h"

//FIXME: 这里后续会改由kmalloc动态分配,这里仅是避免实现kmalloc才固定的,
//若是所有静态变量加上代码超过8MB,那么就要尽快实现kmalloc了
struct block_buf g_bio_buf[BLOCK_BUF_NUM];

struct io_queue {
    struct list_head queue;
    u32 item_cnt;
} g_io_queue;

int bio_init()
{
    memset(g_bio_buf, 0x0, sizeof(g_bio_buf));

    INIT_LIST_HEAD(&g_io_queue.queue);
    g_io_queue.item_cnt = 0;

    return 0;
}

static struct block_buf * bio_get(u32 dev, u32 block_idx)
{
    int i;

    //如果已有缓存直接返回即可
    for (i = 0; i < BLOCK_BUF_NUM; i++) {
        if ((g_bio_buf[i].dev == dev) && g_bio_buf[i].idx == block_idx) {
            g_bio_buf[i].ref_cnt++;
            return &g_bio_buf[i];
        }
    }

    //回收缓冲区, 只要是引用个数为0的均可使用
    for (i = 0; i < BLOCK_BUF_NUM; i++) {
        if (0 == g_bio_buf[i].ref_cnt) {
            g_bio_buf[i].ref_cnt = 1;
            g_bio_buf[i].dev = dev;
            g_bio_buf[i].idx = block_idx;
            g_bio_buf[i].flags = 0;
            memset(g_bio_buf[i].data, 0x0, BLOCK_BUF_SIZE);
            return &g_bio_buf[i];
        }
    }

    return NULL;
}

struct block_buf * bio_read(struct block_dev *dev, u32 block_idx)
{
    struct block_buf *buf = NULL;

    buf = bio_get(dev->dev, block_idx);

    //需要从硬盘中读取的,先加入缓存区队列,然后调用对应块设备读取数据
    if ((NULL != buf) && !BLOCK_IS_VALID(buf)) {
        bio_sched(dev, buf);

        while ((buf->flags & (BLOCK_FLAG_DIRTY | BLOCK_FLAG_VALID)) != BLOCK_FLAG_VALID) {
            /* sleep(buf); */
        }
    }

    return buf;
}


void bio_write(struct block_dev *dev, struct block_buf *buf)
{
    if (NULL == buf) {
        printk("empty buf!\n");
        return ;
    }

    buf->flags |= BLOCK_FLAG_DIRTY;

    bio_sched(dev, buf);

    while ((buf->flags & (BLOCK_FLAG_DIRTY | BLOCK_FLAG_VALID)) != BLOCK_FLAG_VALID) {
        /* sleep(buf); */
    }
}

void bio_free(struct block_buf *buf)
{
    if (NULL == buf || 0 == buf->ref_cnt) {
        printk("params error!\n");
        return ;
    }

    buf->ref_cnt--;

    if (0 == buf->ref_cnt)
        buf->flags = 0;
}

void bio_sched(struct block_dev *dev, struct block_buf *buf)
{
    list_add(&buf->queue, &g_io_queue.queue);
    g_io_queue.item_cnt++;

    if (g_io_queue.item_cnt == 1)
        dev->rw(buf);
}

struct block_buf* bio_front()
{
    struct block_buf *buf;

    if (list_empty(&g_io_queue.queue))
        return NULL;

    buf = list_entry(g_io_queue.queue.next, struct block_buf, queue);
    return buf;
}

struct block_buf* bio_pop()
{
    struct block_buf *buf;

    buf = list_entry(g_io_queue.queue.next, struct block_buf, queue);

    list_del(&buf->queue);
    g_io_queue.item_cnt--;

    return buf;
}
