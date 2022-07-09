#ifndef __TINY_KERNEL_MM_H__
#define __TINY_KERNEL_MM_H__

#include "type.h"
#include "list.h"
#include "page.h"

typedef struct page_allocator_head {
    u32 page_cnt;
    struct list_head head;
} page_allocator_head;

/**
* @brief 页内存分配器初始化 [begin, end)
*
* @param [in/out] page_list 链表头
* @param [in] begin 起始地址，向下对齐
* @param [in] end 结束地址，向上对齐
*
* @return 成功 => TK_STATUS_SUCCESS，失败 => 见具体错误码
*/
TK_STATUS page_allocator_init(page_allocator_head *page_list, uaddr begin, uaddr end);

/**
* @brief 内存分配器分配PAGE_SIZE大小的内存块
*
* @return NULL => 分配失败， != NULL => 分配成功
*/
void *page_allocator_alloc(page_allocator_head *page_list);

/**
* @brief 释放内存
*
* @param [in]addr 目标起始地址，必须为页对齐的内存地址
*/
void page_allocator_free(page_allocator_head *page_list, void *addr);

/**
* @brief 启动时内存分配器初始化
*
* @return 成功 => TK_STATUS_SUCCESS，失败 => 见具体错误码
*/
TK_STATUS bootm_init();

/**
* @brief 内存分配器分配PAGE_SIZE大小的内存块
*
* @return NULL => 分配失败， != NULL => 分配成功
*/
void *bootm_alloc();

/**
* @brief 释放内存
*
* @param [in]addr 目标起始地址，必须为页对齐的内存地址
*/
void bootm_free(void *addr);

/**
* @brief 分配一个页大小的内存
*
* @return NULL => 分配失败， != NULL => 分配成功
*/
void *page_alloc();

/**
* @brief 释放内存
*
* @param [in]addr 目标起始地址，必须为页对齐的内存地址
*/
void page_free(void *addr);

/**
* @brief 内存管理初始化，包括虚拟内存地址映射相关
*
* @return TK_STATUS
*/
TK_STATUS mm_init();

extern int g_mm_initialized;
#endif
