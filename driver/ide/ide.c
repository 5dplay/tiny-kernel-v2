#include "limits.h"
#include "type.h"
#include "common.h"
#include "trap.h"
#include "bio.h"
#include "block_dev.h"
#include "list.h"
#include "string.h"
//#include "sched.h"
#include "x86_trap.h"
#include "x86_asm.h"

// IDE implement ......................
#define SECTOR_SIZE   512
#define IDE_BSY       0x80
#define IDE_DRDY      0x40
#define IDE_DF        0x20
#define IDE_ERR       0x01

#define IDE_CMD_READ  0x20
#define IDE_CMD_WRITE 0x30
#define IDE_CMD_RDMUL 0xc4
#define IDE_CMD_WRMUL 0xc5

#define BSIZE 512
#define FSSIZE 1000
#define SECTOR_SIZE 512

static int havedisk1 = 0;
static void ide_intr(struct trap_frame *frame);
static void ide_rw(struct block_buf *buf);

// Wait for IDE disk to become ready.
int ide_wait(int checkerr)
{
    int r;

    while (((r = inb(0x1f7)) & (IDE_BSY | IDE_DRDY)) != IDE_DRDY)
        ;

    if (checkerr && (r & (IDE_DF | IDE_ERR)) != 0) {
        int err = inb(0x1f1);
        printk("r=%x, err=%x\n", r, err);
        return -1;
    }

    return 0;
}

int ide_init(void)
{
    int i;

    ide_wait(0);

    // Check if disk 1 is present
    outb(0x1f6, 0xe0 | (1 << 4));

    for (i = 0; i < 1000; i++) {
        if (inb(0x1f7) != 0) {
            havedisk1 = 1;
            break;
        }
    }

    // Switch back to disk 0.
    outb(0x1f6, 0xe0 | (0 << 4));

    irq_register(IRQ_IDE, ide_intr);
    irq_enable(IRQ_IDE);

    register_block_dev(ROOTDEV, ide_rw);
    return 0;
}

static void ide_intr(struct trap_frame *frame)
{
    struct block_buf *buf, *next;

    buf = bio_pop();

    if (NULL == buf) {
        printk("error: enter ide_intr without block buffer\n");
        return ;
    }

    if (!BLOCK_IS_DIRTY(buf) && (ide_wait(1) >= 0)) {
        insl(0x1f0, buf->data, BSIZE / 4);
    }

    buf->flags |= BLOCK_FLAG_VALID;
    buf->flags &= ~BLOCK_FLAG_DIRTY;
    //唤醒相关的进程
    /* wakeup(buf); */

    //process next io request
    //TODO: 在中断上下文不要再获取下一个buffer
    if ((next = bio_front()) != NULL) {
        get_block_dev(buf->dev)->rw(next);
    }
}

/**
* @brief process first io request
*/
void ide_do_request(struct block_buf *buf)
{
    int sector ;

    int sector_per_block =  BSIZE / SECTOR_SIZE;
    int read_cmd = (sector_per_block == 1) ? IDE_CMD_READ :  IDE_CMD_RDMUL;
    int write_cmd = (sector_per_block == 1) ? IDE_CMD_WRITE : IDE_CMD_WRMUL;

    sector = buf->idx * sector_per_block;
    ide_wait(0);
    outb(0x3f6, 0x0);  // generate interrupt
    outb(0x1f2, sector_per_block);  // number of sectors
    outb(0x1f3, sector & 0xff);
    outb(0x1f4, (sector >> 8) & 0xff);
    outb(0x1f5, (sector >> 16) & 0xff);
    outb(0x1f6, 0xe0 | ((buf->dev & 1) << 4) | ((sector >> 24) & 0x0f));

    if (buf->flags & BLOCK_FLAG_DIRTY) {
        outb(0x1f7, write_cmd);
        outsl(0x1f0, buf->data, BSIZE / 4);
    } else {
        outb(0x1f7, read_cmd);
    }
}

static void ide_rw(struct block_buf *buf)
{
    if ((buf->flags & (BLOCK_FLAG_DIRTY | BLOCK_FLAG_VALID)) == BLOCK_FLAG_VALID)
        return ;

    ide_do_request(buf);
}
