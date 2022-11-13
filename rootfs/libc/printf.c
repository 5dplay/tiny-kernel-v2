#include <stdarg.h>
#include "include/sys_call.h"
#include "include/libc.h"

static void putc(int fd, char c)
{
    write(fd, &c, 1);
}

static void printint(int fd, int xx, int base, int sgn)
{
    static char digits[] = "0123456789ABCDEF";
    char buf[16] = {0};
    int i, neg;
    unsigned int x;

    neg = 0;

    if (sgn && xx < 0) {
        neg = 1;
        x = -xx;
    } else {
        x = xx;
    }

    i = 0;

    do {
        buf[i++] = digits[x % base];
    } while ((x /= base) != 0);

    if (neg)
        buf[i++] = '-';

    while (--i >= 0)
        putc(fd, buf[i]);
}

// Print to the given fd. Only understands %d, %x, %p, %s.
void printf(const char *fmt, ...)
{
    char *s, char_tmp;
    int c, i, state, fd, tmp;
    va_list args;

    state = 0;
    fd = stdout;
    va_start(args, fmt);

    for (i = 0; fmt[i]; i++) {
        c = fmt[i] & 0xff;

        if (state == 0) {
            if (c == '%') {
                state = '%';
            } else {
                putc(fd, c);
            }
        } else if (state == '%') {
            switch (c) {
                case 'd':
                    tmp = va_arg(args, int);
                    printint(fd, tmp, 10, 1);
                    break;

                case 'x':
                case 'p':
                    tmp = va_arg(args, int);
                    printint(fd, tmp, 16, 1);
                    break;

                case 's':
                    s = va_arg(args, char *);

                    if (s == 0)
                        s = "(null)";

                    while (*s != 0) {
                        putc(fd, *s);
                        s++;
                    }

                    break;

                case 'c':
                    char_tmp = va_arg(args, int);
                    putc(fd, char_tmp);
                    break;

                case '%':
                    putc(fd, c);
                    break;

                default:
                    putc(fd, '%');
                    putc(fd, c);
            }

            state = 0;
        }
    }

    va_end(args);
}
