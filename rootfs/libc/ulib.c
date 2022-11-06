#include "include/sys_call.h"
#include "include/libc.h"

char* strcpy(char *s, const char *t)
{
    char *os;

    os = s;

    while ((*s++ = *t++) != 0)
        ;

    return os;
}

int strcmp(const char *p, const char *q)
{
    while (*p && *p == *q)
        p++, q++;

    return (unsigned char) * p - (unsigned char) * q;
}

int strlen(const char *s)
{
    int n;

    for (n = 0; s[n]; n++)
        ;

    return n;
}

#if defined(x86)
static inline void stosb(void *addr, int data, int cnt)
{
    asm volatile("cld; rep stosb" :
                 "=D"(addr), "=c"(cnt) :
                 "0"(addr), "1"(cnt), "a"(data) :
                 "memory", "cc");
}

void* memset(void *dst, int c, int n)
{
    stosb(dst, c, n);
    return dst;
}
#elif defined(arm)
void* memset(void *dst, int val, int cnt)
{
    char *xs = (char *)dst;

    while (cnt--)
        *xs++ = val;

    return dst;
}

#endif


char* strchr(const char *s, char c)
{
    for (; *s; s++)
        if (*s == c)
            return (char*)s;

    return 0;
}

char* gets(char *buf, int max)
{
    int i, cc;
    char c;

    for (i = 0; i + 1 < max;) {
        cc = read(0, &c, 1);

        if (cc < 1)
            break;

        buf[i++] = c;

        if (c == '\n' || c == '\r')
            break;
    }

    buf[i] = '\0';
    return buf;
}

int stat(const char *n, struct stat *st)
{
    int fd;
    int r;

    fd = open(n, O_RDONLY);

    if (fd < 0)
        return -1;

    r = fstat(fd, st);
    close(fd);
    return r;
}

int atoi(const char *s)
{
    int n;

    n = 0;

    while ('0' <= *s && *s <= '9')
        n = n * 10 + *s++ - '0';

    return n;
}

void* memmove(void *vdst, const void *vsrc, int n)
{
    char *dst;
    const char *src;

    dst = vdst;
    src = vsrc;

    while (n-- > 0)
        *dst++ = *src++;

    return vdst;
}
