#include "memlayout.h"
#include "mm.h"

page_allocator_head s_bootm_pages;
static int s_bootm_inited = 0;

TK_STATUS bootm_init()
{
    TK_STATUS ret;
    ret = page_allocator_init(&s_bootm_pages, (uaddr)tiny_kernel_end,
                              phy_to_virt(PHY_8M));

    if (TK_STATUS_IS_ERR(ret))
        return ret;

    s_bootm_inited = 1;

    return TK_STATUS_SUCCESS;
}
void *bootm_alloc()
{
    if (!s_bootm_inited)
        return NULL;
    else
        return page_allocator_alloc(&s_bootm_pages);
}
void bootm_free(void *addr)
{
    if (s_bootm_inited)
        page_allocator_free(&s_bootm_pages, addr);
}
