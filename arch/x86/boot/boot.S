.section ".text.boot"

.p2align 2
.globl _start
multiboot_header:
#define magic 0x1badb002
#define flags 0
    .long magic
    .long flags
    .long -(magic+flags)

_start:
    movl $kernel_main, %eax
    jmp *%eax

loop:
    hlt
    jmp loop
