#ifndef __TINY_KERNEL_TRAP_H__
#define __TINY_KERNEL_TRAP_H__

#include "type.h"

struct trap_frame;
typedef void (*trap_handler)(struct trap_frame *);

/**
* @brief 异常初始化 - each arch
*
* @return
*/
TK_STATUS trap_init();


/* 中断相关api接口 - begin */

/**
* @brief 中断控制器初始化 - each arch
*
* @return
*/
TK_STATUS irq_init();

/**
* @brief 为相关的中断号irqno注册处理函数
*
* @param irqno 中断号
* @param func 中断处理函数
*/
void irq_register(int irqno, trap_handler func);

/**
* @brief 注销相关的中断号irqno相关的处理函数
*
* @param irqno 中断号
* @param func 中断处理函数
*/
void irq_unregister(int irqno, trap_handler func);

/**
* @brief 允许中断号irqno请求
*
* @param irqno 中断号
*/
void irq_enable(int irqno);

/**
* @brief 禁用中断号irqno请求
*
* @param irqno 中断号
*/
void irq_disable(int irqno);

/**
* @brief 中断处理结束 - FIXME: 是否需要暴露出来？
*/
void irq_end();

/**
* @brief 禁止中断响应
*/
void disable_trap();

/**
* @brief 允许中断响应
*/
void enable_trap();

/* 中断相关api接口 - end */

#endif
