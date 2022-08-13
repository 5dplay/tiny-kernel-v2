# copy from https://gitee.com/suweishuai/baremetal.git
.syntax unified


.irp c,,eq,ne,cs,cc,mi,pl,vs,vc,hi,ls,ge,lt,gt,le,hs,lo
.macro ret\c, reg
.ifeqs "\reg", "lr"
bx\c \reg
.else
mov\c pc, \reg
.endif
.endm
.endr


.macro ARM_DIV2_ORDER divisor, order
clz \order, \divisor
rsb \order, \order, #31
.endm


.macro ARM_DIV_BODY dividend, divisor, result, curbit
clz \curbit, \divisor
clz \result, \dividend
sub \result, \curbit, \result
mov \curbit, #1
mov \divisor, \divisor, lsl \result
mov \curbit, \curbit, lsl \result
mov \result, #0
# 85 "arch/arm/lib/lib1funcs.S"
@ Division loop
1: cmp \dividend, \divisor
subhs \dividend, \dividend, \divisor
orrhs \result, \result, \curbit
cmp \dividend, \divisor, lsr #1
subhs \dividend, \dividend, \divisor, lsr #1
orrhs \result, \result, \curbit, lsr #1
cmp \dividend, \divisor, lsr #2
subhs \dividend, \dividend, \divisor, lsr #2
orrhs \result, \result, \curbit, lsr #2
cmp \dividend, \divisor, lsr #3
subhs \dividend, \dividend, \divisor, lsr #3
orrhs \result, \result, \curbit, lsr #3
cmp \dividend, #0 @ Early termination?
movsne \curbit, \curbit, lsr #4 @ No, any more bits to do?
movne \divisor, \divisor, lsr #4
bne 1b
.endm



.globl __udivsi3 ; .align 0 ; __udivsi3:
.globl __aeabi_uidiv ; .align 0 ; __aeabi_uidiv:

subs r2, r1, #1
reteq lr
bcc Ldiv0
cmp r0, r1
bls 11f
tst r1, r2
beq 12f

ARM_DIV_BODY r0, r1, r2, r3

mov r0, r2
ret lr

11: moveq r0, #1
movne r0, #0
ret lr

12: ARM_DIV2_ORDER r1, r2

mov r0, r0, lsr r2
ret lr

.type __udivsi3, %function; .size __udivsi3, .-__udivsi3
.type __aeabi_uidiv, %function; .size __aeabi_uidiv, .-__aeabi_uidiv



.globl __aeabi_uidivmod ; .align 0 ; __aeabi_uidivmod:

stmfd sp!, {r0, r1, ip, lr}
bl __aeabi_uidiv
ldmfd sp!, {r1, r2, ip, lr}
mul r3, r0, r2
sub r1, r1, r3
ret lr
.type __aeabi_uidivmod, %function; .size __aeabi_uidivmod, .-__aeabi_uidivmod


__div0:
bx	lr

Ldiv0:
str lr, [sp, #-8]!
bl __div0
mov r0, #0 @ About as wrong as it could be.
ldr pc, [sp], #8

.type Ldiv0, %function; .size Ldiv0, .-Ldiv0
