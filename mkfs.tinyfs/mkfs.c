#include "common.h"
#include "io.h"
#include "inode_op.h"
#include "tk_fs.h"

#undef htons
#define htons(val) val


/**
* @brief 构建文件系统镜像时候的上下文
*/
struct mkfs_ctx {
    char *src_dir;              //输入rootfs文件夹路径（相对、绝对路径均可）
    DIR  *src_dir_fd;           //根rootfs对应的句柄
    char *dst_img;              //输出文件路径（相对、绝对路径均可）
    struct superblock_info sb_info;
    struct tk_superblock sb;
} g_mkfs_ctx;

static void show_help()
{
    printf("Usage: mkfs -o fs.img -d rootfs/ \n");
}

/**
* @brief 解析命令行参数
*
* @param [in]argc
* @param [in]argv
* @param [out]ctx 上下文
*
* @return success == 0, failure < 0
*/
static int parse_args(int argc, char **argv, struct mkfs_ctx *ctx)
{
    int opt = 0;
    assert(NULL != ctx);

    while ((opt = getopt(argc, argv, "o:d:")) != -1) {
        switch (opt) {
            case 'o':
                ctx->dst_img = strdup(optarg);
                break;

            case 'd':
                ctx->src_dir = strdup(optarg);
                break;
        }
    }

    if (NULL == ctx->dst_img || NULL == ctx->src_dir)
        return -1;

    return 0;
}

static void mkfs_ctx_init(struct mkfs_ctx *ctx)
{
    assert(NULL != ctx);
    memset(ctx, 0x0, sizeof(*ctx));
    //这里是无奈之举，有空再修正有关的所有函数
    ctx->sb_info.sb = &ctx->sb;
    ctx->sb_info.inode_free = FS_ROOT_INODE;
}

static void mkfs_ctx_sb_init(struct mkfs_ctx *ctx);
static void mkfs_ctx_empty_dst_img(struct mkfs_ctx *ctx);
static int mkfs_ctx_build_rootfs(struct mkfs_ctx *ctx);
static void mkfs_ctx_fill_sb(struct mkfs_ctx *ctx);

/**
* @brief 进行前期的初始化准备
*
* @param [in/out]ctx 上下文
*
* @return success == 0, failure < 0, 除非必要，否则失败后均由上层处理
*/
static int mkfs_ctx_prepare(struct mkfs_ctx *ctx)
{
    assert((NULL != ctx) && (NULL != ctx->src_dir) && (NULL != ctx->dst_img));

    ctx->sb_info.fd = open(ctx->dst_img, O_RDWR | O_CREAT | O_TRUNC, 0666);

    if (ctx->sb_info.fd < 0) {
        perror("Create dst img error: ");
        //call free by mkfs_ctx_uninit;
        return -1;
    }

    ctx->src_dir_fd = opendir(ctx->src_dir);

    if (NULL == ctx->src_dir_fd) {
        perror("Read src dir error: ");
        //call free by mkfs_ctx_uninit;
        return -1;
    }

    //填充超级块基本信息
    mkfs_ctx_sb_init(ctx);

    //填充输出文件镜像为全0
    mkfs_ctx_empty_dst_img(ctx);

    return 0;
}

static void mkfs_ctx_uninit(struct mkfs_ctx *ctx)
{
    assert(NULL != ctx);
    FREE_PTR_SAFE(ctx->dst_img);
    FREE_PTR_SAFE(ctx->src_dir);
    CLOSE_FD_SAFE(ctx->sb_info.fd);
    CLOSE_DIR_SAFE(ctx->src_dir_fd);
}

int main(int argc, char **argv)
{
    int ret = 0;
    printf("tk_inode=%d, tk_dirent=%d\n", sizeof(struct tk_inode), sizeof(struct tk_dirent));
    assert((FS_BLOCK_SIZE % sizeof(struct tk_inode)) == 0);
    assert((FS_BLOCK_SIZE % sizeof(struct tk_dirent)) == 0);

    mkfs_ctx_init(&g_mkfs_ctx);

    if (parse_args(argc, argv, &g_mkfs_ctx) < 0) {
        show_help();
        ret = -1;
        goto free_mkfs_ctx;
    }

    mkfs_ctx_prepare(&g_mkfs_ctx);

    if (mkfs_ctx_build_rootfs(&g_mkfs_ctx) < 0) {
        fprintf(stderr, "failed to build rootfs\n");
        ret = -1;
        goto free_mkfs_ctx;
    }

    mkfs_ctx_fill_sb(&g_mkfs_ctx);

free_mkfs_ctx:
    mkfs_ctx_uninit(&g_mkfs_ctx);
    return ret;
}

static void mkfs_ctx_sb_init(struct mkfs_ctx *ctx)
{
    struct tk_superblock *sb;
    uint32_t off, avail_blocks_cnt;
    assert(NULL != ctx);
    sb = ctx->sb_info.sb;

    sb->magic = 0x1996;
    sb->size = MAX_BLOCK_NUM;
    sb->n_inodes = MAX_INODE_NUM;

    //预留一个boot分区和一个超级块
    //注意此处先不转成网络字节序，等待构建完rootfs，将超级块写回文件的时候再转
    off = LOGS_OFFSET;

    off += FS_MAX_LOG_BLOCKS_NUM;
    sb->inodes_off = off;

    off += MAX_INODE_BLOCK_NUM;
    sb->bitmap_off = off;

    off += MAX_BITMAP_NUM;
    avail_blocks_cnt = MAX_BLOCK_NUM - off;
    sb->n_blocks = avail_blocks_cnt;

    ctx->sb_info.block_free = off;
    block_mark_alloc(&ctx->sb_info, off);

    printf("meta %d (boot, super, inode %lu, bitmap %lu) block %lu, total %lu\n",
           off, MAX_INODE_BLOCK_NUM,
           MAX_BITMAP_NUM, avail_blocks_cnt, MAX_BLOCK_NUM);
    return ;
}

static void mkfs_ctx_empty_dst_img(struct mkfs_ctx *ctx)
{
    uint8_t buf[FS_BLOCK_SIZE];
    int i;
    assert(NULL != ctx);

    memset(buf, 0x0, sizeof(buf));

    for (i = 0; i < ctx->sb.size; i++)
        block_write(ctx->sb_info.fd, i, buf);

    return ;
}

/**
* @brief 创建目录
*
* @param [in]ctx 上下文
* @param [in]dir 待创建目录名
* @param [in]pi_info 父目录
* @param [out]i_info 子目录
*
* @return 目录inode, 第一个创建出来的必然是FS_ROOT_INODE 或者说是 1
*/
static uint32_t mkfs_ctx_mkdir(struct mkfs_ctx *ctx, const char *dir,
                               struct inode_info *pi_info, struct inode_info *i_info)
{
    uint32_t inode;
    struct tk_dirent entry;
    /*
        1、申请一个节点，并且添两个节点 . 和 ..
           其中 . 指向自身，.. 指向上层目录inode，若是根inode则也是指向自己
        2、在上层目录inode添加一个节点，指向自己
    */
    inode_alloc(&ctx->sb_info, TK_STAT_DIR, i_info);
    inode = i_info->idx;

    memset(&entry, 0x0, sizeof(entry));
    entry.i_no = htons(inode);
    strcpy(entry.name, ".");
    inode_append(&ctx->sb_info, i_info, (uint8_t *)&entry, sizeof(entry));

    memset(&entry, 0x0, sizeof(entry));

    if (NULL != pi_info)
        entry.i_no = htons(pi_info->idx);
    else
        entry.i_no = htons(inode);

    strcpy(entry.name, "..");
    inode_append(&ctx->sb_info, i_info, (uint8_t *)&entry, sizeof(entry));
    i_info->info.n_links += 2;

    if (NULL != pi_info) {
        memset(&entry, 0x0, sizeof(entry));
        entry.i_no = htons(inode);
        strcpy(entry.name, dir);
        inode_append(&ctx->sb_info, pi_info, (uint8_t *)&entry, sizeof(entry));

        //逻辑上没关联，待优化相关代码
        pi_info->info.n_links += 1;
    }

    //这里没有必要先刷新一次缓存
    //put_inode_info(&ctx->sb_info, i_info);

    return inode;
}
/**
* @brief 往指定目录添加文件
*
* @param [in]ctx 上下文
* @param [in]path_name 文件名
* @param [in]file_fd 文件句柄
* @param [in]dir_inode 目录inode
*
* @return 文件inode
*/
static uint32_t mkfs_ctx_addfile(struct mkfs_ctx *ctx, const char *path_name,
                                 int file_fd, struct inode_info *dir)
{
    struct inode_info i_info;
    struct tk_dirent entry;
    int cnt;
    uint8_t buf[FS_BLOCK_SIZE];

    inode_alloc(&ctx->sb_info, TK_STAT_FILE, &i_info);
    memset(&entry, 0x0, sizeof(entry));
    entry.i_no = htons(i_info.idx);
    strcpy(entry.name, path_name);
    inode_append(&ctx->sb_info, dir, (uint8_t *)&entry, sizeof(entry));
    i_info.info.n_links = 1;

    while ((cnt = read(file_fd, buf, FS_BLOCK_SIZE)) > 0)
        inode_append(&ctx->sb_info, &i_info, buf, cnt);

    put_inode_info(&ctx->sb_info, &i_info);
    return 0;
}

static int mkfs_ctx_build_rootfs(struct mkfs_ctx *ctx)
{
    assert(NULL != ctx);
//最多容纳100个目录
#define MAX_DIR_QUEUE_NUM 100
//文件路径长度最大1024
#define MAX_PATH_SIZE 1024
    DIR *queue[MAX_DIR_QUEUE_NUM], *cur_dir, *subdir;
    char path[MAX_DIR_QUEUE_NUM][MAX_PATH_SIZE];//队列中目录对应的路径
    struct inode_info inodes[MAX_DIR_QUEUE_NUM], dir_inode;
    char buf[MAX_PATH_SIZE], dir_buf[MAX_PATH_SIZE];
    struct dirent *entry;
    int queue_cnt = 0, fd;
    //层次遍历
    queue[queue_cnt] = ctx->src_dir_fd;
    strcpy(path[queue_cnt], ctx->src_dir);
    mkfs_ctx_mkdir(ctx, ctx->src_dir, NULL, &inodes[queue_cnt]);
    queue_cnt++;

    while (queue_cnt) {
        queue_cnt--;
        cur_dir = queue[queue_cnt];
        dir_inode = inodes[queue_cnt];
        strcpy(dir_buf, path[queue_cnt]);

        while ((entry = readdir(cur_dir)) != NULL) {
            sprintf(buf, "%s/%s", dir_buf, entry->d_name);

            switch (entry->d_type) {
                case DT_DIR:
                    if ((0 != strcmp(".", entry->d_name)) &&
                        (0 != strcmp("..", entry->d_name))) {
                        mkfs_ctx_mkdir(ctx, entry->d_name, &dir_inode, &inodes[queue_cnt]);
                        //FIXME: 异常处理
                        subdir = opendir(buf);
                        queue[queue_cnt] = subdir;
                        strcpy(path[queue_cnt], buf);
                        queue_cnt++;
                    }

                    break;

                case DT_REG:
                    //FIXME: 异常处理
                    fd = open(buf, O_RDONLY);
                    mkfs_ctx_addfile(ctx, entry->d_name, fd, &dir_inode);
                    close(fd);
                    break;

                default:
                    printf("unexpect file %s type %d\n", entry->d_name, entry->d_type);
            }
        }

        if (cur_dir != ctx->src_dir_fd)
            closedir(cur_dir);

        //遍历完当前目录后刷新inode节点到磁盘中
        put_inode_info(&ctx->sb_info, &dir_inode);
    }

    return 0;
}

static void mkfs_ctx_fill_sb(struct mkfs_ctx *ctx)
{
    uint8_t buf[FS_BLOCK_SIZE];
    struct tk_superblock sb;
    assert(NULL != ctx);

    sb = *ctx->sb_info.sb;

    superblock_m2d(ctx->sb_info.sb);

    memset(buf, 0x0, sizeof(buf));
    memcpy(buf, ctx->sb_info.sb, sizeof(*ctx->sb_info.sb));
    block_write(ctx->sb_info.fd, 1, buf);

    superblock_d2m(ctx->sb_info.sb);

    printf("size=%u, n_blocks=%u, n_inodes=%u, inodes_off=%u, bitmap_off=%u\n",
           sb.size, sb.n_blocks, sb.n_inodes, sb.inodes_off, sb.bitmap_off);
}
