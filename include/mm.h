#ifndef __TINY_KERNEL_MM_H__
#define __TINY_KERNEL_MM_H__

#include "type.h"
#include "list.h"
#include "page.h"

#define MAX_ORDER 10
struct page_info {
    u8 order;
    u8 flag;        /*保留字段*/
    int prev_pg;
    int next_pg;
    struct list_head node;
    uaddr vaddr;
};
typedef struct page_allocator_head {
    uaddr addr_base;
    uaddr addr_begin;
    uaddr addr_end;
    int order_list[MAX_ORDER];
    struct page_info *pg_info;
    int pg_info_size;
} page_allocator_head;

/**
* @brief 页内存分配器初始化 [begin, end)
*
* @param [in/out] page_list 链表头
* @param [in] begin 虚拟起始地址，向下对齐
* @param [in] end 虚拟结束地址，向上对齐
*
* @return 成功 => TK_STATUS_SUCCESS，失败 => 见具体错误码
*/
TK_STATUS page_allocator_init(page_allocator_head *page_list, uaddr begin, uaddr end, uaddr base);

/**
* @brief 内存分配器分配PAGE_SIZE * 2^order大小的内存块
*
* @param [in]page_list 链表头
* @param [in]order 2的幂次方个数
*
* @return NULL => 分配失败， != NULL => 分配成功
*/
void *page_allocator_alloc(page_allocator_head *page_list, uint order);

/**
* @brief 释放内存
*
* @param [in]addr 目标起始地址，必须为页对齐的内存地址
* @param [in]order 2的幂次方个数
*/
void page_allocator_free(page_allocator_head *page_list, void *addr, uint order);

/**
* @brief 启动时内存分配器初始化
*
* @return 成功 => TK_STATUS_SUCCESS，失败 => 见具体错误码
*/
TK_STATUS bootm_init();

/**
* @brief 虚拟地址映射完毕时内存分配器初始化
*
* @return 成功 => TK_STATUS_SUCCESS，失败 => 见具体错误码
*/
TK_STATUS page_init();

/* bootm init 之后直接就可以使用page_alloc分配内存，
   不需要额外区分bootm和page内存的内存分配，故屏蔽这一段代码。*/
#if 0
/**
* @brief 内存分配器分配PAGE_SIZE大小的内存块
*
* @return NULL => 分配失败， != NULL => 分配成功
*/
void *bootm_alloc();

/**
* @brief 内存分配器分配PAGE_SIZE * 2^order大小的内存块
*
* @param [in]order 2的幂次方个数
*
* @return NULL => 分配失败， != NULL => 分配成功
*/
void *bootm_alloc_v2(uint order);

/**
* @brief 释放内存
*
* @param [in]addr 目标起始地址，必须为页对齐的内存地址
*/
void bootm_free(void *addr);

/**
* @brief 释放内存
*
* @param [in]addr 目标起始地址，必须为页对齐的内存地址
* @param [in]order 2的幂次方个数
*/
void bootm_free_v2(void *addr, uint order);
#endif

/**
* @brief 分配一个页大小的内存
*
* @return NULL => 分配失败， != NULL => 分配成功
*/
void *page_alloc();

/**
* @brief 内存分配器分配PAGE_SIZE * 2^order大小的内存块
*
* @param [in]order 2的幂次方个数
*
* @return NULL => 分配失败， != NULL => 分配成功
*/
void *page_alloc_v2(uint order);

/**
* @brief 释放内存
*
* @param [in]addr 目标起始地址，必须为页对齐的内存地址
*/
void page_free(void *addr);

/* only for debug page
 * 0 for bootm, 1 for page
 */
void page_dump(int type);

/**
* @brief 释放内存
*
* @param [in]addr 目标起始地址，必须为页对齐的内存地址
* @param [in]order 2的幂次方个数
*/
void page_free_v2(void *addr, uint order);

/**
* @brief 内存管理初始化，包括虚拟内存地址映射相关
*
* @return TK_STATUS
*/
TK_STATUS mm_init();


extern int g_mm_initialized;
#endif
