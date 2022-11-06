#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>
#include <arpa/inet.h>
#include <dirent.h>
#include "tk_fs.h"

#define MAX_BLOCK_NUM 1024
#define MAX_INODE_NUM 100
#define MAX_BITMAP_NUM ((MAX_BLOCK_NUM/FS_BITS_PER_BLOCK)+1)
#define MAX_INODE_BLOCK_NUM ((MAX_BLOCK_NUM/FS_INODE_NUM_PER_BLOCK)+1)

#define LOGS_OFFSET 0x2

/*
    Disk layout:
    **************************************************************
    |boot block|sb block|log|inode blocks|free bitmap|data blocks|
    **************************************************************
*/

#define FREE_PTR_SAFE(ptr) do { \
    if (NULL != (ptr)) {        \
        free(ptr);              \
        (ptr) = NULL;           \
    }                           \
} while (0)

#define CLOSE_FD_SAFE(fd) do {  \
    if ((fd) >= 0) {            \
        close(fd);              \
        (fd) = 0;               \
    }                           \
} while (0)

#define CLOSE_DIR_SAFE(dir) do {    \
    if (NULL != (dir)) {            \
        closedir(dir);              \
        (dir) = NULL;               \
    }                               \
} while (0)

#define MAX(i, j) ((i) > (j) ? (i) : (j))
#define MIN(i, j) ((i) < (j) ? (i) : (j))

#endif
