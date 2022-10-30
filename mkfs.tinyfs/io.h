#ifndef __FS_IO_H__
#define __FS_IO_H__

#include <stdint.h>

/**
* @brief 往指定文件的指定块写入一块数据
*
* @param [in]fd 文件句柄
* @param [in]idx 块号
* @param [in]buf 数据缓冲区 assert(sizeof(buf) == FS_BLOCK_SIZE)
*
* @return success == 0, failure < 0
*/
int block_write(int fd, uint32_t idx, const uint8_t *buf);

/**
* @brief 往指定文件的指定块读取一块数据
*
* @param [in]fd 文件句柄
* @param [in]idx 块号
* @param [out]buf 数据缓冲区 assert(sizeof(buf) == FS_BLOCK_SIZE)
*
* @return success == 0, failure < 0
*/
int block_read(int fd, uint32_t idx, uint8_t *buf);

#endif
