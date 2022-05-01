.section ".text.boot"

.p2align 2
.global _start

_start:
    // shut off extra cores
    mrc p15, 0, r5, c0, c0, 5
    and r5, r5, #3
    cmp r5, #0
    bne halt

    ldr r5, =_start
    mov sp, r5

    ldr r3, =kernel_main
    blx r3

halt:
    wfe
    b halt
