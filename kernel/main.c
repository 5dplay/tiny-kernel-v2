#include "common.h"
#include "type.h"
#include "mm.h"

void kernel_main()
{
    TK_STATUS ret;
    early_print("Hello World!\n");

    ret = bootm_init();

    if (TK_STATUS_IS_OK(ret))
        early_print("success to init bootm!\n");
    else {
        early_print("failed to init bootm!\n");
        goto failed;
    }

    ret = mm_init();

    if (TK_STATUS_IS_OK(ret))
        early_print("success to init mm!\n");
    else {
        early_print("failed to init mm!\n");
        goto failed;
    }

    g_mm_initialized = 1;

failed:

    while (1);
}
