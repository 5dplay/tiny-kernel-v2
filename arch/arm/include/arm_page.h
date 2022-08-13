#ifndef __TINY_KERNEL_ARM_PAGE_H__
#define __TINY_KERNEL_ARM_PAGE_H__

//1MB section mode during boot stage
#define SECTION_SIZE 0x100000
#define SCETION_MASK (~(SCETION_SIZE-1))
#define SCETION_ALIGN_CEIL(addr) (((addr)+SCETION_SIZE-1)&SCETION_MASK)
#define SCETION_ALIGN_FLOOR(addr) ((addr)&SCETION_MASK)

#define SECTION_SHIFT 20
#define SECTION_IDX(addr) (((addr)>>SECTION_SHIFT)&0xFFF)
#define SECTION_ADDR(val) ((val)&SECTION_MASK)

/*

Section:
    ************************************************
    |addr|SBZ|0|nG|S|APX|TEX|AP|P|Domain|XN|C|B|1|0|
    ************************************************

addr:   31-20
SBZ:    19 (should be zero)
nG:     17
S:      16
APX:    15
TEX:    14-12
AP:     11-10
P:      9
Domain: 8-5
XN:     4
C:      3
B:      2

APX/AP: Access Permission bit => 在启动阶段设置为Full Access，即0b011
TEX/C/B: Memory type and cacheable properties => 在启动阶段设置为Normal memory type，即0b 001 0 0
S: Shareable
XN: Execute Never bit => 防止该内存出现执行操作，这里设置为0
Domain: 在armv7被废除的标记位，应当设置为全0，并且将DACR的所有位为Client模式
nG: Not-Global flag => 在启动解决设置为全局的，即为0

已经固定一下的有：SBZ = 0, nG = 0, S = 0, APX = 0, TEX = 000, AP = 11, P = 1, Domain = 0000, XN = 0,
    [31-20] 0000 0000 1110 0000 C/B 10

    ra = vaddr >> 20;
    rb = page_dir[ra]; //page_dir | (ra << 2)

    rb_val = paddr&0xFFF00000 | 0xE00 | flag | 2;
    [rb] = rb_val;

    flag = 0xC for normal, 0x0 for device

*/
#define SECTION_FLAGS 0xc02


/*
    level 1 translation table
    ************************************************
    |level 2 descriptor base   |P|Domain|  SBZ |0|1|
    ************************************************
P:      9
Domain: 8-5
SBZ:    4-2 (should be zero)
    P = 1, Domain = 0000, SBZ = 000
    lvl 2 addr [31-10]
    flag = 0b1000,0000,10 = 0x201
*/
#define PDE_DEF_FLAGS 0x201
/*
    level 2 translation table
    ************************************************
    |Small Page Base Addr |nG|S|APX|TEX|AP|C|B|1|XN|
    ************************************************
nG: 11 Not-Global flag => 在启动解决设置为全局的，即为0
S: 10 Shareable
APX/AP: 9/5-4 Access Permission bit => 默认0b101，即为Privileged下可读
        若是用户态可访问，设置AP = 0b10
        若是可写，设置APX = 0b0
        def = 0x110
        | user = 0x120
        | write = 0x20
TEX/C/B: 8-6/3/2 Memory type and cacheable properties => 在启动阶段设置为Normal memory type，即0b 001 0 0 => 这里似乎有问题，怎么最后设下的时候是Strong-ordered？
XN: Execute Never bit => 防止该内存出现执行操作，这里设置为0
    small page [31-12]
    nG = 0, S = 0, APX = 0, TEX = 000, AP = 00, C = 0, B = 0, XN = 0
    flags = 0x2
*/
#define PTE_DEF_FLAGS 0x2

#define PDE_SHIFT 20
#define PDE_IDX(addr) (((addr)>>PDE_SHIFT)&0xFFF)
#define PDE_ADDR(val) ((val)&PAGE_MASK)

#define PTE_SHIFT 12
#define PTE_IDX(addr) (((addr)>>PTE_SHIFT)&0xFF)
#define PTE_ADDR(val) ((val)&PAGE_MASK)
#define PTE_FLAGS(val) ((val)&(PAGE_SIZE-1))

//pde idx | pte idx | offset
#define GEN_PDE_ADDR(d, t, o) ((uint)((d) << PDE_SHIFT | (t) << PTE_SHIFT | (o)))
#define NPDENTRIES 4096 // # directory entries per page directory

#ifndef MMIO_BASE_PHY
#define MMIO_BASE_PHY 0x3F000000
#endif

#ifndef __ASSEMBLER__
#include "type.h"
struct arm_vmm;
struct page_map_info {
    uaddr pg_dir;
    uaddr vaddr;
    uaddr paddr;
    uint size;
    uint flags;
};
TK_STATUS arm_page_map(struct arm_vmm *this, struct page_map_info *info);
#endif

#endif
