#include "common.h"
#include "io.h"
#include "inode_op.h"

//内存写磁盘前序列化
void inode_m2d(struct tk_inode *info)
{
    int i;
    assert(NULL != info);
    //FIXME: 暂不处理序列化问题
    return ;

    info->type = htons(info->type);
    info->n_links = htons(info->n_links);
    info->file_size = htonl(info->file_size);

    for (i = 0; i < FS_INODE_LN_MAX_NUM; i++)
        info->blocks[i] = htonl(info->blocks[i]);
}

//从磁盘中读出后反序列化
void inode_d2m(struct tk_inode *info)
{
    int i;
    assert(NULL != info);
    //FIXME: 暂不处理序列化问题
    return ;

    info->type = ntohs(info->type);
    info->n_links = ntohs(info->n_links);
    info->file_size = ntohl(info->file_size);

    for (i = 0; i < FS_INODE_LN_MAX_NUM; i++)
        info->blocks[i] = ntohl(info->blocks[i]);
}

void superblock_m2d(struct tk_superblock *sb)
{
    assert(NULL != sb);
    //FIXME: 暂不处理序列化问题
    return ;

    sb->size = htonl(sb->size);
    sb->n_blocks = htonl(sb->n_blocks);
    sb->n_inodes = htonl(sb->n_inodes);
    sb->inodes_off = htonl(sb->inodes_off);
    sb->bitmap_off = htonl(sb->bitmap_off);
}

void superblock_d2m(struct tk_superblock *sb)
{
    assert(NULL != sb);
    //FIXME: 暂不处理序列化问题
    return ;

    sb->size = ntohl(sb->size);
    sb->n_blocks = ntohl(sb->n_blocks);
    sb->n_inodes = ntohl(sb->n_inodes);
    sb->inodes_off = ntohl(sb->inodes_off);
    sb->bitmap_off = ntohl(sb->bitmap_off);
}

int get_inode_info(struct superblock_info *sb_info, uint32_t inode,
                   struct inode_info *i_info)
{
    uint32_t idx;
    struct tk_inode *ibuf;
    static uint8_t buf[FS_BLOCK_SIZE];

    if (NULL == sb_info || 0 == inode || NULL == i_info) {
        fprintf(stderr, "%s wrong args, sb_info=%p, inode=0x%x, i_info=%p\n",
                __func__, sb_info, inode, i_info);
        return -1;
    }

    /*
        1、根据inode和ctx->inodes_off获取对应文件的起始偏移块号
        2、读取该块号数据
        3、填入inode信息
    */
    idx = get_inode_block(sb_info->sb, inode);
    memset(buf, 0x0, sizeof(buf));

    if (block_read(sb_info->fd, idx, buf) < 0) {
        fprintf(stderr, "block read 0x%x failed.\n", idx);
        return -1;
    }

    ibuf = (struct tk_inode *)buf;
    ibuf += (inode % FS_INODE_NUM_PER_BLOCK);
    inode_d2m(ibuf);
    i_info->idx = inode;
    i_info->info = *ibuf;
    return 0;
}

int put_inode_info(struct superblock_info *sb_info, const struct inode_info *i_info)
{
    uint32_t idx;
    struct tk_inode *ibuf;
    static uint8_t buf[FS_BLOCK_SIZE];

    if (NULL == sb_info || NULL == i_info) {
        fprintf(stderr, "%s wrong args, sb_info=%p, i_info=%p\n",
                __func__, sb_info, i_info);
        return -1;
    }

    /*
        1、根据inode和ctx->inodes_off获取对应文件的起始偏移块号
        2、读取该块号数据
        3、填入inode信息
    */
    idx = get_inode_block(sb_info->sb, i_info->idx);
    memset(buf, 0x0, sizeof(buf));

    if (block_read(sb_info->fd, idx, buf) < 0) {
        fprintf(stderr, "block read 0x%x failed.\n", idx);
        return -1;
    }

    ibuf = (struct tk_inode *)buf;
    ibuf += (i_info->idx % FS_INODE_NUM_PER_BLOCK);
    *ibuf = i_info->info;
    inode_m2d(ibuf);
    block_write(sb_info->fd, idx, buf);

    return 0;
}

int inode_alloc(struct superblock_info *sb_info, uint16_t type,
                struct inode_info *i_info)
{
    if (NULL == sb_info || NULL == i_info) {
        fprintf(stderr, "%s wrong args, sb_info=%p, i_info=%p\n",
                __func__, sb_info, i_info);
        return -1;
    }

    /*
        1、获取当前空闲的inode节点
        2、初始化inode的信息, 并且填入
    */
    memset(i_info, 0x0, sizeof(*i_info));
    i_info->idx = sb_info->inode_free++;

    i_info->info.file_size = 0;
    i_info->info.n_links = 0;
    i_info->info.type = type;

    return put_inode_info(sb_info, i_info);
}

int inode_append(struct superblock_info *sb_info, struct inode_info *i_info,
                 const void *data, uint32_t size)
{
    int i, j, tmp, b_no, b_no2;
    uint32_t file_size, cnt;
    struct tk_inode *inode;
    uint8_t buf[FS_BLOCK_SIZE];
    uint8_t disk_buf[FS_BLOCK_SIZE];
    int indirect_write = 0;
    uint32_t *addr;

    if (NULL == sb_info || NULL == i_info || NULL == data || 0 == size) {
        fprintf(stderr, "%s wrong args, sb_info=%p, i_info=%p, data=%p, size=0x%x\n",
                __func__, sb_info, i_info, data, size);
        return -1;
    }

    /*
        1、检查现有剩余空间是否能存放下，不行则申请多一块block
        2、存入当前数据
        3、更新inode节点信息
    */
    file_size = i_info->info.file_size;
    cnt = size;

    inode = &i_info->info;

    while (cnt) {
        i = file_size % FS_BLOCK_SIZE;
        j = file_size / FS_BLOCK_SIZE;

        if (j < FS_INODE_L0_MAX_NUM) {
            if (0 == inode->blocks[j])
                inode->blocks[j] = block_alloc(sb_info);

            b_no = inode->blocks[j];
        } else {
            if (0 == inode->blocks[FS_INODE_L0_MAX_NUM]) {
                b_no2 = inode->blocks[FS_INODE_L0_MAX_NUM] = block_alloc(sb_info);
                //FIXME: 检查返回值
                block_read(sb_info->fd, b_no2, buf);
                indirect_write = 1;
                addr = (uint32_t *)buf;
            }

            j -= FS_INODE_L0_MAX_NUM;

            //FIXME: 检查溢出
            if (0 == addr[j])
                addr[j] = htonl(block_alloc(sb_info));

            b_no = ntohl(addr[j]);
        }

        tmp = MIN(cnt, FS_BLOCK_SIZE - i);
        block_read(sb_info->fd, b_no, disk_buf);
        memcpy(disk_buf + i, data, tmp);
        block_write(sb_info->fd, b_no, disk_buf);
        data += tmp;
        file_size += tmp;
        cnt -= tmp;
    }

    inode->file_size = file_size;

    if (indirect_write)
        block_write(sb_info->fd, b_no2, buf);

    return 0;
}

//注意，这也就是意味着，从boot到superblock其实已经默认被使用了，要想办法解决这个问题
uint32_t block_alloc(struct superblock_info *sb_info)
{
    uint32_t block, i, j, k, idx;
    uint8_t tmp;
    uint8_t buf[FS_BLOCK_SIZE];

    block = sb_info->block_free++;
    idx = get_bitmap_block(sb_info->sb, block);
    block_read(sb_info->fd, idx, buf);

    //TODO: 优化此处，IO过于频繁
    k = block % FS_BITS_PER_BLOCK;

    i = k / 8;
    j = k % 8;
    buf[i] |= (1 << j);

    block_write(sb_info->fd, idx, buf);

    return block;
}

void block_mark_alloc(struct superblock_info *sb_info, uint32_t end)
{
    uint32_t b_no, i, j, k, cnt;
    uint8_t buf[FS_BLOCK_SIZE];
    //FIXME: 这里应该做一些检查
    b_no = sb_info->sb->bitmap_off;

    do {
        memset(buf, 0x0, sizeof(buf));
        cnt = MIN(end, FS_BITS_PER_BLOCK);
        k = j = cnt / 8;

        for (i = 0; i < j; i++)
            buf[i] = 0xFF;

        j = cnt % 8;

        for (i = 0; i < j; i++)
            buf[k] |= (1 << i);

        end -= cnt;
    } while (end > 0);
}
