#ifndef __TINY_KERNEL_PAGE_H__
#define __TINY_KERNEL_PAGE_H__



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
Domain: 在armv7被废除的标记位，应当设置为全0，并且将DACR的所有位为0
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

#ifndef MMIO_BASE_PHY
#define MMIO_BASE_PHY 0x3F000000
#endif

#endif
