/* copy from uboot */
#include "arm_trap.h"
.global arm_vector_start, arm_vector_end

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

.macro save_user_regs
    sub sp, #S_FRAME_SIZE
    stmia sp, {r0 - r12}
    ldr r2, __trap_stack_top
    ldmia r2, {r2 - r3}
    add r0, sp, #S_FRAME_SIZE
    add r5, sp, #S_SP
    mov r1, lr
    stmia r5, {r0 - r3}                 @ save sp_SVC, lr_SVC, pc, cpsr
    mov r0, sp
.endm

__trap_reset:
    bl do_trap_reset

__trap_undef_instruction:
    bl do_undef_instruction

__trap_software_interrupt:
    bl do_software_interrupt

__trap_prefetch_abort:
    bl do_prefetch_abort

__trap_data_abort:
    setup_trap_stack
    save_user_regs
    bl do_data_abort

__trap_not_used:
    bl do_trap_not_used

__trap_irq:
    bl do_irq

__trap_fiq:
    bl do_fiq

arm_vector_end:

trap_stack:
    .fill 0x1000, 2, 0
trap_stack_top:
