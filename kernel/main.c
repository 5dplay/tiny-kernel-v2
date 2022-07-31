#include "common.h"
#include "type.h"
#include "mm.h"
#include "string.h"

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


failed:

    while (1);
}
