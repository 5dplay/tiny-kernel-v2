.globl switch_ctx, arm_forkret_entry

switch_ctx:
    stmfd r13!, {r4-r12, lr}
    str r13, [r0]
    mov r13, r1
    ldmfd r13!, {r4-r12, lr}
    bx lr

arm_forkret_entry:
    @ push {fp, lr}
    bl forkret
    pop {fp, lr}
    mov sp, fp
    bx lr
