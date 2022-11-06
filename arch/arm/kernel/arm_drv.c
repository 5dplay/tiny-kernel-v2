#include "../driver/mini_uart/mini_uart.h"

extern int sd_init();
void drv_init()
{
    mini_uart_init();
    sd_init();
}
