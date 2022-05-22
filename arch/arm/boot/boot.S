#include "memlayout.h"
#include "page.h"

.section ".text.boot"

.p2align 2
.global _start

_start:
    // shut off extra cores
    mrc p15, 0, r5, c0, c0, 5
    and r5, r5, #3
    cmp r5, #0
    bne halt

/*
    初始化页表, 建立虚拟映射关系
    [0, 8M) -> [0, 8M)
    ...
    [KERNEL_BASE, KERNEL_BASE+8M) -> [0, 8M)
    [KERNEL_BASE + 0x3F000000 + PHY_2M) -> [0x3F000000 + PHY_2M)
    TODO: 1. 假若之后的kernel大小超出了8M的限制,此时需要怎么处理?
    armv7 使用1MB页，简化mmu过程
*/ 
setup_mmu:

    ldr r0, LC0
    ldr r2, LC1
    mov r1, #SECTION_FLAGS
mmu_stage0:
    str r1, [r0], #4
    add r1, #SECTION_SIZE

    cmp r0, r2
    blt mmu_stage0

    ldr r0, LC2
    ldr r2, LC3
    mov r1, #SECTION_FLAGS
mmu_stage1:
    str r1, [r0], #4
    add r1, #SECTION_SIZE

    cmp r0, r2
    blt mmu_stage1

/*额外映射MMIO_BASE+PHY_2M的地址处*/
    ldr r0, LC4
    ldr r1, LC5
mmu_stager2:
    str r1, [r0], #4
    add r1, #SECTION_SIZE

mmu_stage3:
    mov r2, #0
    mcr p15, 0, r2, c7, c7, 0
    mcr p15, 0, r2, c8, c7, 0
    mcr p15, 0, r2, c7, c10, 4

    /*设置Domains*/
    mvn r2, #0
    bic r2, #0xc
    mcr p15, 0, r2, c3, c0, 0

    /*设置TTBR*/
    ldr r0, LC0
    mcr p15, 0, r0, c2, c0, 0
    mcr p15, 0, r0, c2, c0, 1

    /*开启MMU功能*/
    mrc p15, 0, r1, c1, c0, 0
    mov r0, #0x1005
    orr r1, r0
    mcr p15, 0, r1, c1, c0, 0

start_kernel:

    ldr r5, LC6
    mov sp, r5

    ldr r3, LC7
    blx r3

LC0:
    .word virt_to_phy(boot_page_dir)
LC1:
    .word virt_to_phy(boot_page_dir)+8*4
LC2:
    .word virt_to_phy(boot_page_dir)+SECTION_IDX(KERNEL_BASE)*4
LC3:
    .word virt_to_phy(boot_page_dir)+SECTION_IDX(KERNEL_BASE+PHY_8M)*4
LC4:
    .word virt_to_phy(boot_page_dir)+SECTION_IDX(KERNEL_BASE+MMIO_BASE_PHY+PHY_2M)*4
LC5:    
    .word MMIO_BASE_PHY+PHY_2M+SECTION_FLAGS
LC6:
    .word boot_kernel_stack_top
LC7:
    .word kernel_main

halt:
    wfe
    b halt

.org 0x1000
boot_kernel_stack:
.org 0x3000
boot_kernel_stack_top:

.org 0x4000

/*页目录必须按16KB对齐*/

boot_page_dir:
    .fill 0x1000, 0x4, 0x0
