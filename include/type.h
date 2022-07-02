#ifndef __TINY_KERNEL_TYPE_H__
#define __TINY_KERNEL_TYPE_H__

#include <stdint.h>

#define NULL ((void *)0)

//FIXME: 仅用32位系统使用以下定义,若是64位请通过#if进行宏控
#define uaddr uintptr_t
#define uint uint32_t
#define u8 uint8_t
#define u16 uint16_t
#define u32 uint32_t

typedef enum TK_STATUS {
    TK_STATUS_SUCCESS       =  0,
    TK_STATUS_FAILURE       = -1,
    TK_STATUS_NO_MEM        = -2,
    TK_STATUS_BAD_ARGS      = -3,
} TK_STATUS;

#define TK_STATUS_IS_OK(ret) ((ret) == TK_STATUS_SUCCESS)
#define TK_STATUS_IS_ERR(ret) ((ret) != TK_STATUS_SUCCESS)

#endif
