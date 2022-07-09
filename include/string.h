#ifndef __TINY_KERNEL_STRING_H__
#define __TINY_KERNEL_STRING_H__

#include <stdarg.h>
#include "type.h"

/**
* @brief 显式带可变参数格式化字符串
*
* @param [out]buf 输出缓冲区
* @param [in]fmt 字符串内容
* @param [in]args 可变参数
*
* @return 格式化后字符串长度
*/
int vsprintf(char *buf, const char *fmt, va_list args);

/**
* @brief 隐式带可变参数格式化字符串
*
* @param [out]buf 输出缓冲区
* @param [in]fmt 字符串内容
* @param [in]... 可变参数
*
* @return 格式化后字符串长度
*/
int sprintf(char * buf, const char *fmt, ...);

/* 常用字符串操作函数 */
void memset(void *dst, u8 val, uint cnt);

int memcmp(const void *s1, const void *s2, uint cnt);

void memcpy(void *dst, const void *src, uint cnt);

#define memmove memcpy

int strncmp(const char *s1, const char *s2, uint cnt);

int strncpy(char *dst, const char *src, uint cnt);

int strncpy_s(char *dst, const char *src, uint cnt);

int strlen(const char *s);

#endif
