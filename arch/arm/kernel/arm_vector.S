/* copy from uboot */
#include "arm_trap.h"
.global arm_vector_start, arm_vector_end, trapret

.section ".text.vector"
arm_vector_start:
    ldr pc, __trap_reset_word
    ldr pc, __trap_undef_instruction_word
    ldr pc, __trap_software_interrupt_word
    ldr pc, __trap_prefetch_abort_word
    ldr pc, __trap_data_abort_word
    ldr pc, __trap_not_used_word
    ldr pc, __trap_irq_word
    ldr pc, __trap_fiq_word

__trap_reset_word: .word __trap_reset
__trap_undef_instruction_word: .word __trap_undef_instruction
__trap_software_interrupt_word: .word __trap_software_interrupt
__trap_prefetch_abort_word: .word __trap_prefetch_abort_word
__trap_data_abort_word: .word __trap_data_abort
__trap_not_used_word: .word __trap_not_used
__trap_irq_word: .word __trap_irq
__trap_fiq_word: .word __trap_fiq
__trap_stack_top: .word trap_stack_top


.macro setup_trap_stack
    ldr r13, __trap_stack_top           @ r13 == sp

    str lr, [r13]                       @ save origin pc，lr == pc
    mrs lr, spsr
    str lr, [r13, #4]                   @ save spsr
    mov r13, #MODE_SVC
    msr spsr, r13
    mov lr, pc
    movs pc, lr
.endm

.macro setup_trap_stack_svc
    sub r13, #4
    str r0, [r13]                       @ save r0
    ldr r0, __trap_stack_top
    str lr, [r0]                        @ save caller lr in position 0
                                        @ of saved stack
    mrs lr, spsr                        @ get the spsr
    str lr, [r0, #4]                    @ save spsr in position 1
                                        @ of saved stack
    ldr lr, [r0]
    ldr r0, [r13]
    add r13, #4
.endm

.macro save_user_regs
    sub sp, #S_FRAME_SIZE
    stmia sp, {r0 - r12}                @ store multi reg to [Rn]
    ldr r2, __trap_stack_top
    ldmia r2, {r2 - r3}                 @ load multi reg from [Rn]
    add r0, sp, #S_FRAME_SIZE
    add r5, sp, #S_SP
    mov r1, lr
    stmia r5, {r0 - r3}                 @ save sp_SVC, lr_SVC, pc, cpsr
    mov r0, sp
.endm

.macro save_user_regs_svc
    sub sp, #S_FRAME_SIZE
    stmia sp, {r0 - r12}                @ store multi reg to [Rn]
    add r5, sp, #S_SP
    mov r2, lr
    mrs r3, spsr
    stmia r5!, {sp, lr}^
    stmia r5!, {r2-r3}
    mov r0, sp
.endm

.macro restore_user_regs_svc
    ldmia sp, {r0 - lr}^
    ldr lr, [sp, #S_PSR]
    msr spsr, lr
    ldr lr, [sp, #S_PC]
    add sp, #S_FRAME_SIZE
    movs pc, lr
.endm

.macro irq_save_user_regs
    sub sp, #S_FRAME_SIZE
    stmia sp, {r0 - r12}
    add r8, sp, #S_PC
    stmdb r8, {sp, lr}^
    str lr, [r8, #0]
    mrs r6, spsr
    str r6, [r8, #4]
    str r6, [r8, #8]
    mov r0, sp
.endm

.macro irq_restore_user_regs
    ldmia sp, {r0 - lr}^
    mov r0, r10
    ldr lr, [sp, #S_PC]
    add sp, #S_FRAME_SIZE
    subs pc, lr, #4
.endm

__trap_reset:
    bl do_trap_reset

__trap_undef_instruction:
    bl do_undef_instruction

__trap_software_interrupt:
    @ setup_trap_stack_svc
    @ save_user_regs
    save_user_regs_svc
    bl do_software_interrupt
trapret:
    restore_user_regs_svc
loop:
    b loop 

__trap_prefetch_abort:
    bl do_prefetch_abort

__trap_data_abort:
    setup_trap_stack
    save_user_regs
    bl do_data_abort

__trap_not_used:
    bl do_trap_not_used

__trap_irq:
    ldr r13, __trap_stack_top           @ r13 == sp
    irq_save_user_regs
    bl do_irq
    irq_restore_user_regs

__trap_fiq:
    ldr r13, __trap_stack_top           @ r13 == sp
    bl do_fiq

arm_vector_end:

trap_stack:
    .fill 0x1000, 2, 0
trap_stack_top:
