//#include "file_system.h"
#include "common.h"
#include "limits.h"
#include "tinyfs.h"
#include "bio.h"
#include "block_dev.h"
#include "string.h"

struct tinyfs_superblock tinyfs_superb;
static struct tinyfs_inode g_inode_cached[NINODE];

static struct inode* tinyfs_geti(struct superblock *superb, u32 idx);
static struct inode* tinyfs_alloci(struct superblock *superb, u16 type, u16 major, u16 minor);
static int tinyfs_meta_readi(struct superblock *superb, struct inode *ip);
static int tinyfs_meta_writei(struct superblock *superb, struct inode *ip);
static struct inode* tinyfs_lookupi(struct superblock *superb, struct inode *src, const char *dname, int size);
static int tinyfs_data_readi(struct superblock *superb, struct inode *ip, u8 *dst, int off, int cnt);
static int tinyfs_data_writei(struct superblock *superb, struct inode *ip, u8 *src, int off, int cnt);
static int tinyfs_data_linki(struct superblock *superb, struct inode *dst, struct inode *src, const char *dname, int size);

static u32 bzero(struct tinyfs_superblock *superb, int block_idx);
static u32 balloc(struct tinyfs_superblock *superb);
static u32 bmap(struct tinyfs_superblock *superb, struct tinyfs_inode *this, u32 bn);
static struct superblock_op tinyfs_op = {
    .geti = tinyfs_geti,
    .alloci = tinyfs_alloci,
    .meta_readi = tinyfs_meta_readi,
    .meta_writei = tinyfs_meta_writei,
    .lookupi = tinyfs_lookupi,
    .data_readi = tinyfs_data_readi,
    .data_writei = tinyfs_data_writei,
    .data_linki = tinyfs_data_linki
};

struct superblock* fs_init(int dev)
{
    struct block_dev *b_dev;
    struct block_buf *buf;
    struct tinyfs_superblock *this;
    struct tinyfs_superblock_disk superb_disk;
    struct inode *ip;

    memset(&tinyfs_superb, 0x0, sizeof(tinyfs_superb));
    this = &tinyfs_superb;
    this->parent.dev = dev;
    this->parent.op = &tinyfs_op;

    b_dev = get_block_dev(dev);
    buf = bio_read(b_dev, 1);
    memcpy(&superb_disk, buf->data, sizeof(superb_disk));

    if (superb_disk.magic != TINY_FS_MAGIC)
        return NULL;

    this->size = superb_disk.size;
    this->n_blocks = superb_disk.n_blocks;
    this->n_inodes = superb_disk.n_inodes;
    this->inode_off = superb_disk.inode_off;
    this->bitmap_off = superb_disk.bitmap_off;
#if 1
    printk("super_block: size=%u, n_blocks=%u, n_inodes=%u, inodes_off=%u, bitmap_off=%u\n",
           this->size, this->n_blocks, this->n_inodes,
           this->inode_off, this->bitmap_off);
#endif
    //FIXME: 这里没必要调用外面的函数
    ip = tinyfs_geti((struct superblock *)this, ROOTINO);
    tinyfs_meta_readi((struct superblock *)this, ip);
    this->parent.root = (struct inode *)ip;

    return (struct superblock *)this;
}

static struct inode* tinyfs_geti(struct superblock *superb, u32 idx)
{
    struct tinyfs_inode *ip, *item, *slot;
    struct inode *tmp;
    int i;

    slot = item = NULL;

    for (i = 0, ip = NULL; i < NINODE; i++) {
        ip = &g_inode_cached[i];

        if (ip->parent.ref && ip->parent.dev == superb->dev && ip->parent.idx == idx) {
            ip->parent.ref++;
            item = ip;
            break;
        }

        if (slot == NULL && ip->parent.ref == 0)
            slot = ip;
    }

    if (!item && slot) {
        item = slot;
        tmp = &item->parent;
        tmp->dev = superb->dev;
        tmp->idx = idx;
        tmp->major = 0;
        tmp->minor = 0;
        tmp->superb = superb;
        tmp->type = 0;
    }

    return (struct inode *)item;
}

static struct inode* tinyfs_alloci(struct superblock *superb, u16 type, u16 major, u16 minor)
{
    //FIXME: 后续都要考虑加锁问题
    struct block_buf *buf;
    struct block_dev *dev;
    struct tinyfs_inode_disk *id;
    struct inode *ip;
    struct tinyfs_superblock *this;
    int i;

    this = (struct tinyfs_superblock *)superb;
    dev = get_block_dev(superb->dev);

    //依次遍历寻找到第一个空闲节点,填充信息,并且生成对应的inode内存节点并且返回
    for (i = 1; i < this->n_inodes; i++) {
        buf = bio_read(dev, IBLOCK(i, this));
        id = (struct tinyfs_inode_disk *)buf->data + (i % IPB);

        if (id->type == 0) {
            memset(id, 0x0, sizeof(*id));
            id->type = type;
            id->major = major;
            id->minor = minor;
            //n_link 由link动作再进行操作
            bio_write(dev, buf);
            ip = tinyfs_geti(superb, i);
            tinyfs_meta_readi(superb, ip);
#if 0
            printk("alloc inode idx = %d, type = %u, major = %u, minor = %u\n", i, type, major, minor);
#endif
            return ip;
        }
    }

    return NULL;
}

static int tinyfs_meta_readi(struct superblock *superb, struct inode *ip)
{
    struct block_buf *buf;
    struct block_dev *dev;
    struct tinyfs_inode_disk *id;
    struct tinyfs_inode *dst;
    struct tinyfs_superblock *this;
    int i;

    this = (struct tinyfs_superblock *)superb;
    dst = (struct tinyfs_inode *)ip;

    dev = get_block_dev(ip->dev);
    buf = bio_read(dev, IBLOCK(ip->idx, this));

    id = (struct tinyfs_inode_disk *)buf->data + ip->idx % IPB;
    dst->parent.type = id->type;
    dst->parent.major = id->major;
    dst->parent.minor = id->minor;
    dst->parent.n_link = id->n_link;
    dst->parent.size = id->size;
    dst->parent.sync = 1;
#if 0
    printk("inode=%u, type=%u, major=%u, minor=%u, n_links=%u, file_size=%u\n",
           dst->parent.idx, dst->parent.type, dst->parent.major,
           dst->parent.minor, dst->parent.n_link, dst->parent.size);
#endif

    for (i = 0; i < NDIRECT + 1; i++)
        dst->addr[i] = id->addr[i];

    return 0;
}

static int tinyfs_meta_writei(struct superblock *superb, struct inode *ip)
{
    struct block_buf *buf;
    struct block_dev *dev;
    struct tinyfs_inode_disk *id;
    struct tinyfs_inode *tip;
    struct tinyfs_superblock *this;
    int i;

    this = (struct tinyfs_superblock *)superb;
    tip = (struct tinyfs_inode *)ip;
    dev = get_block_dev(ip->dev);
    buf = bio_read(dev, IBLOCK(ip->idx, this));

    id = (struct tinyfs_inode_disk *)buf->data + ip->idx % IPB;
    id->type = tip->parent.type;
    id->major = tip->parent.major;
    id->minor = tip->parent.minor;
    id->n_link = tip->parent.n_link;
    id->size = tip->parent.size;

    for (i = 0; i < NDIRECT + 1; i++)
        id->addr[i] = tip->addr[i];

    bio_write(dev, buf);

    return 0;
}

static struct inode* tinyfs_lookupi(struct superblock *superb, struct inode *src, const char *dname, int size)
{
    u32 off;
    struct tinyfs_dirent_disk de;
    struct inode *ip;

#if 0
    printk("%s: read inode %d\n", __func__, this->parent.idx);
#endif

    for (off = 0; off < src->size; off += sizeof(de)) {
        //FIXME: 判断返回值错误等等
        tinyfs_data_readi(superb, src, (uint8_t *)&de, off, sizeof(de));

        if (de.idx == 0)
            continue;

#if 0
        printk("this entry %s: %d\n", de.name, de.idx);
#endif

        if (strncmp(dname, de.name, size) == 0) {
            ip = tinyfs_geti(superb, de.idx);
            tinyfs_meta_readi(superb, ip);
            return ip;
        }
    }

    return NULL;

}

static int tinyfs_data_readi(struct superblock *superb, struct inode *ip, u8 *dst, int off, int cnt)
{
    u32 sum, tmp;
    struct block_buf *buf;
    struct block_dev *dev;

    if (off > ip->size || off + cnt < off)
        return -1;

    if (off + cnt > ip->size)
        cnt = ip->size - off;


    dev = get_block_dev(ip->dev);

    for (sum = 0; sum < cnt; sum += tmp, off += tmp, dst += tmp) {
        buf = bio_read(dev, bmap(superb, ip, off / BSIZE));
        tmp = BSIZE - (off % BSIZE);

        if (tmp > cnt - sum)
            tmp = cnt - sum;

        memcpy(dst, buf->data + (off % BSIZE), tmp);
    }

    return sum;
}

static int tinyfs_data_writei(struct superblock *superb, struct inode *ip, u8 *src, int off, int cnt)
{
    u32 sum, tmp;
    struct block_buf *buf;
    struct block_dev *dev;

    if (off > ip->size || off + cnt < off)
        return -1;

    if (off + cnt > ip->size)
        cnt = ip->size - off;

    dev = get_block_dev(ip->dev);

    for (sum = 0; sum < cnt; sum += tmp, off += tmp, src += tmp) {
        buf = bio_read(dev, bmap(superb, ip, off / BSIZE));
        tmp = BSIZE - (off % BSIZE);

        if (tmp > cnt - sum)
            tmp = cnt - sum;

        memcpy(buf->data + (off % BSIZE), src, tmp);
        bio_write(dev, buf);
    }

    return sum;
}

static int tinyfs_data_linki(struct superblock *superb, struct inode *dst, struct inode *src, const char *dname, int size)
{
    u32 off;
    struct tinyfs_dirent_disk de;

    memset(&de, 0x0, sizeof(de));

    for (off = 0; off < dst->size; off += sizeof(de)) {
        tinyfs_data_readi(superb, dst, (uint8_t *)&de, off, sizeof(de));

        if (de.idx == 0)
            break;
    }

    //FIXME: 扩容这个步骤究竟在哪里做？
    if (off >= dst->size)
        dst->size += sizeof(de);

    strncpy(de.name, dname, sizeof(de.name));
    de.idx = src->idx;
    tinyfs_data_writei(superb, dst, (uint8_t *)&de, off, sizeof(de));

    dst->n_link++;
    return 0;

}

static u32 bzero(struct tinyfs_superblock *superb, int block_idx)
{
    struct block_buf *buf;
    struct block_dev *dev;

    dev = get_block_dev(superb->parent.dev);

    buf = bio_read(dev, block_idx);
    memset(buf->data, 0x0, BSIZE);
    bio_write(dev, buf);
    return 0;

}

static u32 balloc(struct tinyfs_superblock *superb)
{
    struct block_buf *buf;
    struct block_dev *dev;
    int i, j, bit;

    dev = get_block_dev(superb->parent.dev);

    for (i = 0; i < superb->size; i += BPB) {
        buf = bio_read(dev, BBLOCK(i, superb));

        for (j = 0; j < BPB && (i + j) < superb->size; j++) {
            bit = 1 << (j % 8);

            if ((buf->data[j / 8] & bit) == 0) {
                buf->data[j / 8] |= bit;
            }

            bzero(superb, i + j);
            return i + j;
        }
    }

    printk("no free block.\n");
    return 0;
}

static u32 bmap(struct tinyfs_superblock *superb, struct tinyfs_inode *this, u32 bn)
{
    struct block_buf *buf;
    struct block_dev *dev;
    u32 *addr;

    if (bn < NDIRECT) {
        if (this->addr[bn] == 0)
            this->addr[bn] = balloc(superb);

        return this->addr[bn];
    }

    bn -= NDIRECT;

    if (bn > NINDIRECT) {
        panic("something wrong with bn=%u\n", bn);
    }

    dev = get_block_dev(this->parent.dev);

    if (this->addr[NDIRECT] == 0)
        this->addr[NDIRECT] = balloc(superb);

    buf = bio_read(dev, this->addr[NDIRECT]);

    addr = (u32 *)buf->data;

    if (addr[bn] == 0)
        addr[bn] = balloc(superb);

    return addr[bn];
}
