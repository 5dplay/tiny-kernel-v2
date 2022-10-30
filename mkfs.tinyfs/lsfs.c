#include "common.h"
#include "io.h"
#include "inode_op.h"

#undef ntohs
#define ntohs(val) val

static void printf_inode(struct superblock_info *sb_info, struct inode_info *i_info)
{
    uint32_t file_size, cnt, i, j, k;
    uint8_t buf[FS_BLOCK_SIZE];
    struct tk_dirent *dirent;
    printf("inode=%u, type=%u, major=%u, minor=%u, n_links=%u, file_size=%u\n",
           i_info->idx, i_info->info.type, i_info->info.major, i_info->info.minor,
           i_info->info.n_links, i_info->info.file_size);
    file_size = i_info->info.file_size;
    i = 0;

    while (file_size && i < FS_INODE_L0_MAX_NUM && i_info->info.blocks[i]) {
        block_read(sb_info->fd, i_info->info.blocks[i], buf);
        cnt = MIN(file_size, FS_BLOCK_SIZE);

        if (i_info->info.type == TK_STAT_DIR) {
            dirent = (struct tk_dirent *)buf;
            k = cnt / sizeof(struct tk_dirent);

            for (j = 0; j < k; j++, dirent++)
                printf("dir entry %s: %u\n", dirent->name, ntohs(dirent->i_no));

        } else if (i_info->info.type == TK_STAT_FILE) {
            k = cnt;

            for (j = 0; j < k; j++) {
                printf("%02X ", buf[j]);

                if (((j + 1) % 16) == 0)
                    printf("\n");
            }

            if ((j % 16) != 0)
                printf("\n");
        }

        file_size -= cnt;
        i++;
    }
}

int main(int argc, char **argv)
{
    struct superblock_info sb_info;
    struct tk_superblock sb;
    struct inode_info i_info;
    uint8_t buf[FS_BLOCK_SIZE];
    uint32_t i;
    char cmd[12];
    uint32_t inode;

    sb_info.sb = &sb;

    if (argc != 2) {
        printf("wrong args\n");
        return -1;
    }

    sb_info.fd = open(argv[1], O_RDONLY, 0666);

    if (sb_info.fd < 0) {
        perror("open img error: ");
        return -1;
    }

    memset(buf, 0x0, sizeof(buf));
    block_read(sb_info.fd, 1, buf);
    memcpy(&sb, buf, sizeof(sb));

    superblock_d2m(&sb);

    printf("size=%u, n_blocks=%u, n_inodes=%u, inodes_off=%u, bitmap_off=%u\n",
           sb.size, sb.n_blocks, sb.n_inodes, sb.inodes_off, sb.bitmap_off);

    printf("> ");

    while (scanf("%s %d", cmd, &inode) != 0) {
        if (strcmp("di", cmd) == 0) {
            get_inode_info(&sb_info, inode, &i_info);
            printf_inode(&sb_info,  &i_info);
        } else if ((strcmp("exit", cmd) == 0) || (strcmp("q", cmd) == 0)) {
            break;
        } else {
            printf("unkown cmd: %s\n", cmd);
        }

        printf("> ");
    }

    close(sb_info.fd);
}
