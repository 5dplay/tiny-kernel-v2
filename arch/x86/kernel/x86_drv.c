#include "type.h"
#include "../driver/keyboard/keyboard.h"

extern int ide_init();

void drv_init()
{
    kbd_init();
    ide_init();
}
