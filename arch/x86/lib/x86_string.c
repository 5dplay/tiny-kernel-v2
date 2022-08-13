#include "string.h"
#include "x86_asm.h"

void memset(void *dst, u8 val, uint cnt)
{
    if ((((uintptr_t)dst % 4) == 0) && ((cnt % 4) == 0)) {
        uint new_val = val;
        stosl(dst, (new_val << 24) | (new_val << 16) | (new_val << 8) | new_val, cnt / 4);
    } else
        stosb(dst, val, cnt);
}
