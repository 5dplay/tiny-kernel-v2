#ifndef __TINY_KERNEL_COMMON_H__
#define __TINY_KERNEL_COMMON_H__

#include "type.h"

#ifdef EARLY_PRINT
void early_print(const char *);
#else
static inline void early_print(const char *str)
{
    /* nothing */
}

#endif

#if 0
void register_console(void (*proc)(const char *msg));
#endif

/**
* @brief 打印信息到终端屏幕上, 在printk.c定义, copy from linux
*
* @param [in]fmt 字符串内容
* @param [in]... 可变参数
*
* @return 格式化后字符串长度
*
*/
#if 1
int printk(const char *fmt, ...);
#else
static inline int printk(const char *fmt, ...)
{
    return 0;
}
#endif

/**
* @brief 打印信息到终端屏幕上, 并且宕机, 在panic.c定义, copy from linux
*
* @param [in]fmt 字符串内容
* @param [in]... 可变参数
*/
//FIXME: 暂时在printk.c里面定义,后续有必要再抽出来
#if 1
void panic(const char *fmt, ...);
#else
static inline void panic(const char *fmt, ...)
{
    while (1);
}
#endif

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))

uint ffs(uint val);

/*copy from linux-2.4.0 arm*/
#define do_div(n, base)                     \
({                                          \
    int __res;                              \
    __res = ((uint)n) % ((uint)base);       \
    n = ((uint)n) / ((uint)base);           \
    __res;                                  \
})

#endif
