#include "common.h"
#include "memlayout.h"
#include "type.h"
#include "mm.h"
#include "string.h"
#include "trap.h"
#include "drv.h"
#include "proc.h"
#include "bio.h"
#include "block_dev.h"

void kernel_main()
{
    TK_STATUS ret;
    printk("Hello World!\n");

    ret = bootm_init();

    if (TK_STATUS_IS_OK(ret))
        printk("success to init bootm!\n");
    else {
        printk("failed to init bootm!\n");
        goto failed;
    }

    ret = mm_init();

    if (TK_STATUS_IS_OK(ret))
        printk("success to init mm!\n");
    else {
        printk("failed to init mm!\n");
        goto failed;
    }

    g_mm_initialized = 1;

    ret = page_init();

    if (TK_STATUS_IS_OK(ret))
        printk("success to init page!\n");
    else {
        printk("failed to init page!\n");
        goto failed;
    }

    /*至此，不适用bootm分配的内存了，原先已用掉的就放着吧，也先不处理了*/
    trap_init();

    irq_init();

    enable_trap();

    bio_init();

    block_dev_init();

    drv_init();

    proc_init();

    scheduler();

    printk("reach here\n");
failed:

    while (1);
}
