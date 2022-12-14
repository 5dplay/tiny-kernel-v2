#include "common.h"
#include "string.h"

#define is_digit(c) ((c) >= '0' && (c) <= '9')
/**
* @brief 跳过整数
*
* @param [in/out] s 字符串指针, 执行完毕后指向第一个非整数的字符
*
* @return 整数
*/
static int skip_atoi(const char **s)
{
    int i = 0;

    while (is_digit(**s)) {
        i = i * 10 + **s - '0';
        (*s)++;
    }

    return i;
}

#define ZEROPAD 0x1
#define SIGN    0x2
#define PLUS	4		/* show plus */
#define SPACE	8		/* space if plus */
#define LEFT	16		/* left justified */
#define SPECIAL	32		/* 0x */
#define SMALL	64		/* use 'abcdef' instead of 'ABCDEF' */

/**
* @brief 按照进制数填充整数字符串,并且返回新的字符串位置
*
* @param [out]str 字符串
* @param [in]num 待格式化的整数
* @param [in]base 进制
* @param [in]size 占位宽带
* @param [in]precision 精度
* @param [in]type 属性
*
* @return
*/
static char * number(char * str, int num, int base, int size, int precision, int type)
{
    char c, sign, tmp[36];
    const char *digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    int i;

    if (type & SMALL) digits = "0123456789abcdefghijklmnopqrstuvwxyz";

    if (type & LEFT) type &= ~ZEROPAD;

    if (base < 2 || base > 36)
        return 0;

    c = (type & ZEROPAD) ? '0' : ' ' ;

    if (type & SIGN && num < 0) {
        sign = '-';
        num = -num;
    } else
        sign = (type & PLUS) ? '+' : ((type & SPACE) ? ' ' : 0);

    if (sign) size--;

    if (type & SPECIAL) {
        if (base == 16) size -= 2;
        else if (base == 8) size--;
    }

    i = 0;

    if (num == 0)
        tmp[i++] = '0';
    else while (num != 0)
            tmp[i++] = digits[do_div(num, base)];

    if (i > precision) precision = i;

    size -= precision;

    if (!(type & (ZEROPAD + LEFT)))
        while (size-- > 0)
            *str++ = ' ';

    if (sign)
        *str++ = sign;

    if (type & SPECIAL) {
        if (base == 8)
            *str++ = '0';
        else if (base == 16) {
            *str++ = '0';
            *str++ = digits[33];
        }
    }

    if (!(type & LEFT))
        while (size-- > 0)
            *str++ = c;

    while (i < precision--)
        *str++ = '0';

    while (i-- > 0)
        *str++ = tmp[i];

    while (size-- > 0)
        *str++ = ' ';

    return str;
}

int vsprintf(char *buf, const char *fmt, va_list args)
{
    int len;
    int i;
    char * str;
    char *s;
    int *ip;

    int flags;		/* flags to number() */

    int field_width;	/* width of output field */
    int precision;		/* min. # of digits for integers; max
				   number of chars for from string */

    for (str = buf ; *fmt ; ++fmt) {
        if (*fmt != '%') {
            *str++ = *fmt;
            continue;
        }

        /* process flags */
        flags = 0;
repeat:
        ++fmt;		/* this also skips first '%' */

        switch (*fmt) {
            case '-':
                flags |= LEFT;
                goto repeat;

            case '+':
                flags |= PLUS;
                goto repeat;

            case ' ':
                flags |= SPACE;
                goto repeat;

            case '#':
                flags |= SPECIAL;
                goto repeat;

            case '0':
                flags |= ZEROPAD;
                goto repeat;
        }

        /* get field width */
        field_width = -1;

        if (is_digit(*fmt))
            field_width = skip_atoi(&fmt);
        else if (*fmt == '*') {
            /* it's the next argument */
            field_width = va_arg(args, int);

            if (field_width < 0) {
                field_width = -field_width;
                flags |= LEFT;
            }
        }

        /* get the precision */
        precision = -1;

        if (*fmt == '.') {
            ++fmt;

            if (is_digit(*fmt))
                precision = skip_atoi(&fmt);
            else if (*fmt == '*') {
                /* it's the next argument */
                precision = va_arg(args, int);
            }

            if (precision < 0)
                precision = 0;
        }

        switch (*fmt) {
            case 'c':
                if (!(flags & LEFT))
                    while (--field_width > 0)
                        *str++ = ' ';

                *str++ = (unsigned char) va_arg(args, int);

                while (--field_width > 0)
                    *str++ = ' ';

                break;

            case 's':
                s = va_arg(args, char *);

                if (!s)
                    s = "<NULL>";

                len = strlen(s);

                if (precision < 0)
                    precision = len;
                else if (len > precision)
                    len = precision;

                if (!(flags & LEFT))
                    while (len < field_width--)
                        *str++ = ' ';

                for (i = 0; i < len; ++i)
                    *str++ = *s++;

                while (len < field_width--)
                    *str++ = ' ';

                break;

            case 'o':
                str = number(str, va_arg(args, unsigned long), 8,
                             field_width, precision, flags);
                break;

            case 'p':
                if (field_width == -1) {
                    field_width = 8;
                    flags |= ZEROPAD;
                }

                str = number(str,
                             (unsigned long) va_arg(args, void *), 16,
                             field_width, precision, flags);
                break;

            case 'x':
                flags |= SMALL;

            case 'X':
                str = number(str, va_arg(args, unsigned long), 16,
                             field_width, precision, flags);
                break;

            case 'd':
            case 'i':
                flags |= SIGN;

            case 'u':
                str = number(str, va_arg(args, unsigned long), 10,
                             field_width, precision, flags);
                break;

            case 'n':
                ip = va_arg(args, int *);
                *ip = (str - buf);
                break;

            default:
                if (*fmt != '%')
                    *str++ = '%';

                if (*fmt)
                    *str++ = *fmt;
                else
                    --fmt;

                break;
        }
    }

    *str = '\0';
    return str - buf;
}

int sprintf(char * buf, const char *fmt, ...)
{
    va_list args;
    int i;

    va_start(args, fmt);
    i = vsprintf(buf, fmt, args);
    va_end(args);
    return i;
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

#ifndef HAVE_ARCH_MEMSET
void memset(void *dst, u8 val, uint cnt)
{
    char *xs = (char *)dst;

    while (cnt--)
        *xs++ = val;
}
#endif
