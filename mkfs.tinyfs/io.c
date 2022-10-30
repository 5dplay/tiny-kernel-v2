#include "common.h"
#include "io.h"

int block_write(int fd, uint32_t idx, const uint8_t *buf)
{
    int ret;
    ret = lseek(fd, idx * FS_BLOCK_SIZE, SEEK_SET);

    if (ret != (idx * FS_BLOCK_SIZE)) {
        perror("lseek: ");
        return -1;
    }

    ret = write(fd, buf, FS_BLOCK_SIZE);

    if (ret != FS_BLOCK_SIZE) {
        perror("write: ");
        return -1;
    }

    return 0;
}
int block_read(int fd, uint32_t idx, uint8_t *buf)
{
    int ret;
    ret = lseek(fd, idx * FS_BLOCK_SIZE, SEEK_SET);

    if (ret != (idx * FS_BLOCK_SIZE)) {
        perror("lseek: ");
        return -1;
    }

    ret = read(fd, buf, FS_BLOCK_SIZE);

    if (ret != FS_BLOCK_SIZE) {
        perror("read: ");
        return -1;
    }

    return 0;
}
