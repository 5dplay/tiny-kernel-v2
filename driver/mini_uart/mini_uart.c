/*
 * Copyright (C) 2018 bzt (bztsrc@github)
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 */

#include "arm_iomap.h"
#include "trap.h"
#include "arm_trap.h"
#include "common.h"
#include "mini_uart.h"
#include "vfs.h"

/* Auxilary mini UART registers */
#define AUX_ENABLE      ((volatile unsigned int*)(MMIO_BASE+0x00215004))
#define AUX_MU_IO       ((volatile unsigned int*)(MMIO_BASE+0x00215040))
#define AUX_MU_IER      ((volatile unsigned int*)(MMIO_BASE+0x00215044))
#define AUX_MU_IIR      ((volatile unsigned int*)(MMIO_BASE+0x00215048))
#define AUX_MU_LCR      ((volatile unsigned int*)(MMIO_BASE+0x0021504C))
#define AUX_MU_MCR      ((volatile unsigned int*)(MMIO_BASE+0x00215050))
#define AUX_MU_LSR      ((volatile unsigned int*)(MMIO_BASE+0x00215054))
#define AUX_MU_MSR      ((volatile unsigned int*)(MMIO_BASE+0x00215058))
#define AUX_MU_SCRATCH  ((volatile unsigned int*)(MMIO_BASE+0x0021505C))
#define AUX_MU_CNTL     ((volatile unsigned int*)(MMIO_BASE+0x00215060))
#define AUX_MU_STAT     ((volatile unsigned int*)(MMIO_BASE+0x00215064))
#define AUX_MU_BAUD     ((volatile unsigned int*)(MMIO_BASE+0x00215068))

#define AUX_MU_IIR_TX(val) ((val) & 0x2)
#define AUX_MU_IIR_RX(val) ((val) & 0x4)

/* also see https://github.com/futurehomeno/RPI_mini_UART.git */


void mini_uart_trap_handler(struct trap_frame *tf)
{
    u32 irq_status;

    irq_status = *AUX_MU_IIR;

    /* only care abot rx */
    if (!AUX_MU_IIR_RX(irq_status))
        return ;

    console_intr();
}

/**
 * Set baud rate and characteristics (115200 8N1) and map to GPIO
 */
void mini_uart_init()
{
    register unsigned int r;

    /* initialize UART */
    *AUX_ENABLE |= 1;      // enable UART1, AUX mini mini_uart
    *AUX_MU_CNTL = 0;
    *AUX_MU_LCR = 3;       // 8 bits
    *AUX_MU_MCR = 0;
    *AUX_MU_IER = 0xD;     // only care about rx interrupt
    *AUX_MU_IIR = 0xC6;    // flush fifo
    *AUX_MU_BAUD = 270;    // 115200 baud
    /* map UART1 to GPIO pins */
    r = *GPFSEL1;
    r &= ~((7 << 12) | (7 << 15)); // gpio14, gpio15
    r |= (2 << 12) | (2 << 15); // alt5
    *GPFSEL1 = r;
    *GPPUD = 0;            // enable pins 14 and 15
    r = 150;

    while (r--) {
        asm volatile("nop");
    }

    *GPPUDCLK0 = (1 << 14) | (1 << 15);
    r = 150;

    while (r--) {
        asm volatile("nop");
    }

    *GPPUDCLK0 = 0;        // flush GPIO setup
    *AUX_MU_CNTL = 3;      // enable Tx, Rx

    irq_register(IRQ_AUX, mini_uart_trap_handler);
    irq_enable(IRQ_AUX);
}

/**
 * Send a character
 */
void mini_uart_send(unsigned int c)
{
    /* wait until we can send */
    do {
        asm volatile("nop");
    } while (!(*AUX_MU_LSR & 0x20));

    /* write the character to the buffer */
    *AUX_MU_IO = c;
}

/**
 * Receive a character
 */
#define AUX_MU_STAT_HAS_RX(val) ((val) & 0xF0000)
#define AUX_RX_BUF_SIZE 8
void mini_uart_getc()
{
    char buf[AUX_RX_BUF_SIZE + 1];
    u32 i, val;

    buf[AUX_RX_BUF_SIZE] = '\0';
    i = 0;

    do {
        val = *AUX_MU_STAT;

        if (!AUX_MU_STAT_HAS_RX(val))
            break;

        buf[i++] = (char)(*AUX_MU_IO);
    } while (i < AUX_RX_BUF_SIZE);

    for (val = 0; val < i; val++) {
        switch (buf[val]) {
            case '\x7f':
                mini_uart_send(0x8);
                mini_uart_send(' ');
                mini_uart_send(0x8);
                break;

            case '\r':
                mini_uart_send('\r');
                mini_uart_send('\n');
                break;

            /* TODO: 仍存在别的控制字符未处理 */
            default:
                mini_uart_send(buf[val]);
        }
    }
}

int mini_uart_getc_console()
{
    int ch;
    u32 val;

    val = *AUX_MU_STAT;

    if (!AUX_MU_STAT_HAS_RX(val))
        return -1;

    ch = (char)(*AUX_MU_IO);

    switch (ch) {
        case '\x7f':
            mini_uart_send(0x8);
            mini_uart_send(' ');
            mini_uart_send(0x8);
            return ch;

        case '\r':
        default:
            return ch;
    }
}

/**
 * Display a string
 */
void mini_uart_puts(const char *s)
{
    while (*s) {
        /* convert newline to carrige return + newline */
        if (*s == '\n')
            mini_uart_send('\r');

        mini_uart_send(*s++);
    }
}

void console_putc(int c)
{
    mini_uart_send(c);
}

int console_getc()
{
    return mini_uart_getc_console();
}
