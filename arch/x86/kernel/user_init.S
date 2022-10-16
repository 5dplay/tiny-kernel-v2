#include "sys_call.h"

#exec(init, argv)
.globl start
start:
    movl $SYS_setup, %eax
    int $0x40
    pushl $argv
    pushl $init
    pushl $0
    movl $SYS_exec, %eax
    int $0x40

exit:
    movl $SYS_exit, %eax
    int $0x40

#never reach
idle:
    jmp idle

#char init[] = "/init\0"
init:
    .string "/init\0"

#char *argv[] = {init, NULL}
.p2align 2
argv:
    .long init
    .long 0
