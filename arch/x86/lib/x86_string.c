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

int memcmp(const void *s1, const void *s2, uint cnt)
{
    const uint8_t *v1, *v2;
    v1 = s1;
    v2 = s2;

    while (cnt--) {
        if (*v1 != *v2)
            return *v1 - *v2;

        v1++;
        v2++;
    }

    return 0;
}

void memcpy(void *dst, const void *src, uint n)
{
    const char *s;
    char *d;

    s = src;
    d = dst;

    if (s < d && s + n > d) {
        s += n;
        d += n;

        while (n-- > 0)
            *--d = *--s;
    } else
        while (n-- > 0)
            *d++ = *s++;
}

int strncmp(const char *s1, const char *s2, uint cnt)
{
    while (cnt && *s1 && *s1 == *s2)
        cnt--, s1++, s2++;

    if (cnt == 0)
        return 0;
    else
        return *s1 - *s2;
}

int strncpy(char *dst, const char *src, uint cnt)
{
    uint org = cnt;

    while (cnt && (*dst++ = *src++))
        cnt--;

    while (cnt)
        cnt--, *dst++ = 0;

    return org - cnt;
}

int strncpy_s(char *dst, const char *src, uint cnt)
{
    uint org = cnt;

    while (--cnt && (*dst++ = *src++));

    *dst = 0;
    return org - cnt;
}

int strlen(const char *str)
{
    int n = 0;

    while (str[n] != 0)
        n++;

    return n;
}

