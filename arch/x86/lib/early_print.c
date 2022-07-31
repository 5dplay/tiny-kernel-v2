#include <stddef.h>
#include <stdint.h>
#include "memlayout.h"
#include "string.h"
#include "x86_asm.h"

#define CRTPORT 0x3D4
#define BACKSPACE 0x100
#define VGA_BUFFER phy_to_virt(0xB8000)

static uint16_t *crt = (uint16_t *)VGA_BUFFER;

void vgaputc(int c)
{
    int pos;

    // Cursor position: col + 80*row.
    outb(CRTPORT, 14);
    pos = inb(CRTPORT + 1) << 8;
    outb(CRTPORT, 15);
    pos |= inb(CRTPORT + 1);

    if (c == '\n')
        pos += 80 - pos % 80;
    else if (c == BACKSPACE) {
        if (pos > 0) --pos;
    } else
        crt[pos++] = (c & 0xff) | 0x0700; // black on white

    if (pos < 0 || pos > 25 * 80) {
        //echo "pos under/overflow"
        return ;
    }

    if ((pos / 80) >= 24) { // Scroll up.
        memcpy(crt, crt + 80, sizeof(crt[0]) * 23 * 80);
        pos -= 80;
        memset(crt + pos, 0, sizeof(crt[0]) * (24 * 80 - pos));
    }

    outb(CRTPORT, 14);
    outb(CRTPORT + 1, pos >> 8);
    outb(CRTPORT, 15);
    outb(CRTPORT + 1, pos);
    crt[pos] = ' ' | 0x0700;
}

void early_print(const char *msg)
{
    int c;

    while ((c = *msg++) != '\0')
        vgaputc(c);
}
