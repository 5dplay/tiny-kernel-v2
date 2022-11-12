#include "vfs.h"
#include "common.h"
#include "proc.h"

#define INPUT_BUF 128
struct {
    u8 buf[INPUT_BUF];
    uint r;     //read index
    uint w;     //write index
    uint e;     //edit index
} input;
#define C(x) ((x)-'@') //Control-x
#define BACKSPACE 0x100

static int console_read(struct inode *ip, char* dst, int cnt)
{
    int target;
    char ch;

    target = cnt;

    while (cnt > 0) {
        while (input.r == input.w) {
            sleep(&input.r);
        }

        ch = input.buf[input.r++ % INPUT_BUF];

        if (ch == C('D')) {
            if (cnt < target) {
                input.r--;
            }

            break;
        }

        *dst++ = ch;
        cnt--;

        if (ch == '\n')
            break;
    }

    return target - cnt;
}

static int console_write(struct inode *ip, char* src, int cnt)
{
    int i;

    for (i = 0; i < cnt; i++)
        console_putc(src[i]);

    return 0;
}

int console_init()
{
    devsw[CONSOLE].read = console_read;
    devsw[CONSOLE].write = console_write;

    return 0;
}

void console_intr()
{
    int c;

    while ((c = console_getc()) >= 0) {
        switch (c) {
            case C('U'):    //kill line
                while (input.e != input.w && input.buf[(input.e-1) % INPUT_BUF] != '\n') {
                    input.e--;
                    console_putc(BACKSPACE);
                }

                break;

            case C('H'):
            case '\x7f': // backspace
                if (input.e != input.w) {
                    input.e--;
                    console_putc(BACKSPACE);
                }

                break;

            default:
                if (c != 0 && input.e-input.r < INPUT_BUF) {
                    c = (c == '\r') ? '\n' : c;
                    input.buf[input.e++ % INPUT_BUF] = c;
                    console_putc(c);

                    if (c == '\n' || c == C('D') || input.e == input.r + INPUT_BUF) {
                        input.w = input.e;
                        wakeup(&input.r);
                    }
                }
        }
    }
}
