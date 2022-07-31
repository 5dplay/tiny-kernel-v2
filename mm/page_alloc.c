#include "common.h"
#include "list.h"
#include "mm.h"
#include "type.h"
#include "memlayout.h"
#include "string.h"

/* 统一bootm和page */
page_allocator_head g_tk_pages;

TK_STATUS bootm_init()
{
    return page_allocator_init(&g_tk_pages,
                               (uaddr)tiny_kernel_end,
                               phy_to_virt(PHY_8M), KERNEL_BASE);
}

void *page_alloc()
{
    return page_allocator_alloc(&g_tk_pages, 0);
}

void *page_alloc_v2(uint order)
{
    return page_allocator_alloc(&g_tk_pages, order);
}

void page_free(void *addr)
{
    page_allocator_free(&g_tk_pages, addr, 0);
}

void page_free_v2(void *addr, uint order)
{
    page_allocator_free(&g_tk_pages, addr, order);
}

/*
    这是一个简单的的页内存管理器,采用buddy的分配策略
*/
#define PGADDR2IDX(addr, base) (((addr)-(base))/PAGE_SIZE)
#define IDX2PGADDR(idx, base) (((idx)*PAGE_SIZE)+(base))
#define BUDDY_IDX(idx, order) ((idx) ^ (1 << (order)))

static void  __add_free_page(page_allocator_head *page_list, uint idx, uint order);
static void __add_free_page_block(page_allocator_head *page_list, uint idx, uint order);
static void __page_allocator_init(page_allocator_head *page_list);

TK_STATUS page_allocator_init(page_allocator_head *page_list, uaddr begin, uaddr end, uaddr base)
{
    uaddr u_start, u_end, tmp;
    int i, cnt;

    if (page_list == NULL || begin == 0 || end == 0)
        return TK_STATUS_BAD_ARGS;

    page_list->addr_base = base;
    u_start = PAGE_ALIGN_CEIL(begin);
    page_list->addr_end = u_end = PAGE_ALIGN_FLOOR(end);

    for (i = 0; i < MAX_ORDER; i++)
        page_list->order_list[i] = -1;

    cnt = (u_end - base) / PAGE_SIZE;

    page_list->pg_info = (struct page_info *)u_start;
    page_list->pg_info_size = cnt;

    page_list->addr_begin = u_start = PAGE_ALIGN_CEIL(u_start + sizeof(struct page_info) * cnt);

    printk("%s: start = %x, end = %x, cnt = %d\n", __func__, u_start, u_end, cnt);

    memset(page_list->pg_info, 0x0, sizeof(struct page_info) * cnt);

    for (i = 0, tmp = base; i < cnt && tmp < u_end; i++, tmp += PAGE_SIZE)
        page_list->pg_info[i].vaddr = tmp;

    __page_allocator_init(page_list);

    return TK_STATUS_SUCCESS;
}

void * page_allocator_alloc(page_allocator_head *page_list, uint order)
{
    int i, idx;
    struct page_info *pg;

    if (page_list == NULL) {
        printk("%s: bad args %p\n", page_list);
        return NULL;
    }

    /*
        buddy 分配逻辑：
            1、从order开始寻找，找到第一个合适的页块
            2、分割页块，将多余的页块（按2的幂次从大到小依次分割）放置到各级空闲链表中
    */
    for (i = order; i < MAX_ORDER; i++)
        if (page_list->order_list[i] != -1)
            break;

    if (i == MAX_ORDER) {
        printk("out of page, check!\n");
        return NULL;
    }

    idx = page_list->order_list[i];
    pg = &page_list->pg_info[idx];
    page_list->order_list[i] = pg->next_pg;
    pg->next_pg = -1;

    while (i-- > order)
        __add_free_page_block(page_list, BUDDY_IDX(idx, i), i);

    pg->flag = 0x1; /* mark used */
    return (void *)pg->vaddr;
}

void page_allocator_free(page_allocator_head *page_list, void *addr, uint order)
{
    uaddr page = (uaddr)addr;

    if (page < page_list->addr_begin || page > page_list->addr_end) {
        printk("addr %x not in range!", page);
        return ;
    }

    if ((page % PAGE_SIZE) != 0) {
        printk("addr %x must be aligned to page size %d!", page, PAGE_SIZE);
        page = PAGE_ALIGN_FLOOR(page);
    }

    __add_free_page(page_list, PGADDR2IDX(page, page_list->addr_base), order);
}

static void __add_free_page(page_allocator_head *page_list,
                            uint idx, uint order)
{
    struct page_info *buddy_pg;
    int i, j;
    uint buddy_idx;
    int end_idx;

    end_idx = PGADDR2IDX(page_list->addr_end, page_list->addr_base);

    while (order < MAX_ORDER) {
        buddy_idx = idx ^ (1 << order);

        if (buddy_idx > end_idx) {
            __add_free_page_block(page_list, idx, order);
            return ;
        }

        for (i = page_list->order_list[order]; (i != -1) && (i != buddy_idx); i = page_list->pg_info[i].next_pg);

        if (i != buddy_idx) {
            __add_free_page_block(page_list, idx, order);
            return ;
        } else {
            buddy_pg = &page_list->pg_info[buddy_idx];

            if (buddy_pg->prev_pg == -1) {
                i = page_list->order_list[order] = buddy_pg->next_pg;
                page_list->pg_info[i].prev_pg = -1;
            } else {
                i = buddy_pg->prev_pg;
                j = buddy_pg->next_pg;
                page_list->pg_info[i].next_pg = j;
                page_list->pg_info[j].prev_pg = i;
            }

            idx = min(idx, buddy_idx);
        }

        order++;
    }
}

static void __add_free_page_block(page_allocator_head *page_list,
                                  uint idx, uint order)
{
    int tmp, i, j;
    struct page_info *pg_info;

    pg_info = &page_list->pg_info[idx];

    tmp = page_list->order_list[order];
    page_list->order_list[order] = idx;
    pg_info->next_pg = tmp;
    pg_info->prev_pg = -1;
    page_list->pg_info[tmp].prev_pg = idx;

    for (i = 0, j = (1 << order); i < j; i++)
        page_list->pg_info[idx + i].order = order;

}

static void __page_allocator_init(page_allocator_head *page_list)
{
    int start_idx, end_idx;
    uint order;

    start_idx = PGADDR2IDX(page_list->addr_begin, page_list->addr_base);
    end_idx = PGADDR2IDX(page_list->addr_end, page_list->addr_base);

    printk("start_idx 0x%x, end_idx 0x%x\n", start_idx, end_idx);

    while (start_idx < end_idx) {
        order = min(MAX_ORDER - 1, ffs(start_idx));

        while ((start_idx + (1 << order)) > end_idx)
            order--;

        __add_free_page_block(page_list, start_idx, order);
        start_idx += (1 << order);
    }

}

void page_dump()
{
    int i, j;
    page_allocator_head *page_list;

    page_list = &g_tk_pages;

    for (j = 0; j < MAX_ORDER; j++) {
        printk("order %d: ", j);

        for (i = page_list->order_list[j]; i != -1; i = page_list->pg_info[i].next_pg)
            printk(" %p", page_list->pg_info[i].vaddr);

        printk("\n");
    }
}
