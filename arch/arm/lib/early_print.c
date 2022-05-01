#include "uart.h"

void early_print(const char *str)
{
    static int first = 1;

    if (first) {
        uart_init();
        first = 0;
    }

    uart_puts(str);
}
