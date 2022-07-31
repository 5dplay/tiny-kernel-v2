#include "common.h"

uint ffs(uint val)
{
    uint ret = 0;

    if (!val)
        return ret;

    if (!(val & 0xFFFF)) {
        val >>= 16;
        ret += 16;
    }

    if (!(val & 0xFF)) {
        val >>= 8;
        ret += 8;
    }

    if (!(val & 0xF)) {
        val >>= 4;
        ret += 4;
    }

    if (!(val & 0x3)) {
        val >>= 2;
        ret += 2;
    }

    if (!(val & 0x1)) {
        val >>= 1;
        ret += 1;
    }

    return ret;
}
