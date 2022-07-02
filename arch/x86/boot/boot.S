#include "memlayout.h"
#include "x86_page.h"

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

/*
    TODO:  这里假设三件事情, 
           1) 开启了A20,
           2) cs = ds = es = ss = gs = 0
           3) 设备信息已经固定了, 参考xv6的实现中关于memlayout部分的内容
           若是以后实现BootLoader,或者需要根据硬件信息进行特定的初始化,需要更改以下部分的代码 
*/


/*
    初始化页表, 建立虚拟映射关系
    [0, 4M) -> [0, 4M)
    [4M, 8M) -> [4M, 8M)
    ...
    [KERNEL_BASE, KERNEL_BASE+4M) -> [0, 4M)
    [KERNEL_BASE+4M, KERNEL_BASE+8M) -> [4M, 8M)
    TODO: 1. 假若之后的kernel大小超出了8M的限制,此时需要怎么处理?

    具体实现:
          1. 预先填充好[0, 8M)的PTE
          2. 填充PDE
*/ 

init_boot_page_stage0:
    cld                     #清除方向标记位, 迭代后edi += 4. std则相反edi -= 4.
    movl $(virt_to_phy(boot_page_entry)), %edi
    movl $0x3, %eax         #PTE项 = (0x0<<12) | 0x3, present + write/read

init_boot_page_stage1:
    stosl                   #[es:edi] <- eax, edi += 4
    add $(PAGE_SIZE), %eax
    cmp $(virt_to_phy(boot_page_entry_end)), %edi
    jne init_boot_page_stage1

init_boot_page_stage2:
    movl $(virt_to_phy(boot_page_entry)), %eax                      #设置[0, 4MB)
    orl $0x3, %eax          #PDE项的值
    movl $(virt_to_phy(boot_page_dir)+PDE_IDX(PHY_0M)*4), %edi
    movl %eax, (%edi)
    movl $(virt_to_phy(boot_page_dir)+PDE_IDX(KERNEL_BASE)*4), %edi
    movl %eax, (%edi)
    
    movl $(virt_to_phy(boot_page_entry)+PAGE_SIZE), %eax            #设置[4MB, 8MB)
    orl $0x3, %eax          #PDE项的值
    movl $(virt_to_phy(boot_page_dir)+PDE_IDX(PHY_4M)*4), %edi
    movl %eax, (%edi)
    movl $(virt_to_phy(boot_page_dir)+PDE_IDX(KERNEL_BASE+PHY_4M)*4), %edi
    movl %eax, (%edi)

enable_paging:
    movl $(virt_to_phy(boot_page_dir)), %eax
    movl %eax, %cr3
    movl %cr0, %eax
    orl $0x80000000, %eax
    movl %eax, %cr0

    movl $(boot_kernel_stack + BOOT_KERNEL_STACK_SIZE), %esp

    movl $kernel_main, %eax             #长跳转, 否则编译器会优化成相对跳转
    jmp *%eax

loop:
    hlt
    jmp loop
.org 0x1000
boot_page_dir:
    .fill 1024, 4, 0

#页表项必须页对齐, 页目录无此限制
.org 0x2000
boot_page_entry:

.org 0x4000
boot_page_entry_end:

.comm boot_kernel_stack, BOOT_KERNEL_STACK_SIZE
