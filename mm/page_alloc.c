#include "common.h"
#include "mm.h"
#include "type.h"

/*
    这是一个简单的的页内存管理器,采用FIFO的策略
*/

/* 空闲页节点 */
typedef struct page_allocator_node {
    u32 magic;              /* 魔数 == BOOTM_PAGE_ALLOC_MAGIC */
    struct list_head node;  /* 链表节点 */
} page_allocator_node;

#define PAGE_ALLOC_MAGIC 0xABCDDCBA

TK_STATUS page_allocator_init(page_allocator_head *page_list, uaddr begin, uaddr end)
{
    uaddr u_start, u_end;
    page_allocator_node *page_desc = NULL;

    if (page_list == NULL || begin == 0 || end == 0)
        return TK_STATUS_BAD_ARGS;

    INIT_LIST_HEAD(&page_list->head);
    page_list->page_cnt = 0;

    u_start = PAGE_ALIGN_CEIL(begin);
    u_end = PAGE_ALIGN_FLOOR(end);

    printk("%s: start = %x, end = %x\n", __func__, u_start, u_end);

    for (; u_start < u_end; u_start += PAGE_SIZE) {
        page_desc = (page_allocator_node *)u_start;
        page_desc->magic = PAGE_ALLOC_MAGIC;
        list_add(&page_desc->node, &page_list->head);
        page_list->page_cnt++;
    }

    return TK_STATUS_SUCCESS;
}

void *page_allocator_alloc(page_allocator_head *page_list)
{
    page_allocator_node *page_desc;
    struct list_head *node;

    if (page_list == NULL) {
        printk("%s: bad args %p\n", page_list);
        return NULL;
    }

    if (page_list->page_cnt == 0) {
        printk("no more free page!!!\n");
        return NULL;
    }

    node = page_list->head.next;
    list_del_init(node);

    page_desc = list_entry(node, page_allocator_node, node);
    return page_desc;
}

void page_allocator_free(page_allocator_head *page_list, void *addr)
{
    uaddr page = (uaddr)addr;
    page_allocator_node *page_desc = NULL;

    if ((page % PAGE_SIZE) != 0) {
        printk("addr %x must be aligned to page size %d!", page, PAGE_SIZE);
        page = PAGE_ALIGN_FLOOR(page);
    }

    page_desc = (page_allocator_node *)page;
    page_desc->magic = PAGE_ALLOC_MAGIC;
    list_add(&page_desc->node, &page_list->head);
    page_list->page_cnt++;
}
