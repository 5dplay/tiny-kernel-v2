#include "x86_seg.h"
.globl alltraps
alltraps:
    pushl %ds
    pushl %es
    pushl %fs
    pushl %gs
    pushal

    movw $(SEG_KDATA<<3), %ax
    movw %ax, %ds
    movw %ax, %es

    pushl %esp
    call trap_svr
    addl $4, %esp

.globl trapret
trapret:
    popal
    popl %gs
    popl %fs
    popl %es
    popl %ds
    addl $8, %esp   #trap_no and errcode
    iret
