#include "sys_call.h"

# exec(init, argv)
.globl start
start:
    mov r7, #SYS_setup
    svc #0
    ldr r0, =init
    ldr r1, =argv
    mov r7, #SYS_exec
    svc #0
exit:
    mov r7, #SYS_exit
    svc #0

#never reach
idle:
    bl idle

#char init[] = "/init\0";
init:
    .string "/init\0"

#char *argv[] = {init, NULL}
.p2align 2
argv:
    .word init
    .word 0
