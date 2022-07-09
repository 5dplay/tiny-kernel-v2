#ifndef __TINY_KERNEL_X86_SEG_H__
#define __TINY_KERNEL_X86_SEG_H__

#ifndef __ASSEMBLER__

#include "type.h"

struct seg_desc {
    u32 limit1 : 16;
    u32 base1 : 16;
    u32 base2 : 8;
    u32 access : 8;
    u32 limit2 : 4;
    u32 flags : 4;
    u32 base3 : 8;
};

struct gdtr {
    uint16_t size;  //the size of table in bytes subtracted by 1
    u32 offset;
} __attribute__((packed));

struct seg_desc_info {
    u32 base;          //A 32-bit value containing the linear address where the segment begins.
    u32 limit;         //A 20-bit value, tells the maximum addressable unit, either in 1 byte units, or in 4KiB pages. Hence, if you choose page granularity and set the Limit value to 0xFFFFF the segment will span the full 4 GiB address space in 32-bit mode.
    uint8_t access;
    uint8_t flags;
};

struct task_state {
    u32 link;
    u32 esp0;
    u16 ss0;
    u16 pad1;
    u32 esp1;
    u16 ss1;
    u16 pad2;
    u32 esp2;
    u16 ss2;
    u16 pad3;
    u32 cr3;
    u32 eip;
    u32 eflags;
    u32 eax;
    u32 ecx;
    u32 edx;
    u32 ebx;
    u32 esp;
    u32 ebp;
    u32 esi;
    u32 edi;
    u16 es;
    u16 pad4;
    u16 cs;
    u16 pad5;
    u16 ss;
    u16 pad6;
    u16 ds;
    u16 pad7;
    u16 fs;
    u16 pad8;
    u16 gs;
    u16 pad9;
    u16 ldt;
    u16 pad10;
    u16 t;
    u16 iomb;
} __attribute__((packed));

#endif //__ASSEMBLER__

#define SEG_DESC_LIMIT1(val) ((val)&0xFFFF)
#define SEG_DESC_LIMIT2(val) (((val)>>16)&0xF)
#define SEG_DESC_BASE1(val) ((val)&0xFFFF)
#define SEG_DESC_BASE2(val) (((val)>>16)&0xFF)
#define SEG_DESC_BASE3(val) (((val)>>24)&0xFF)
#define SEC_DESC_FLAGS(val) ((val)&0xF)

#define SEG_NULL  0  //not used
#define SEG_KCODE 1 //kernel code
#define SEG_KDATA 2 //kernel data
#define SEG_UCODE 3 //user code
#define SEG_UDATA 4 //user data
#define SEG_TSS   5
#define SEG_MAX   6

/*
*********************************
|SEG|Pr|Privl|S|Ex|DC|RW|Ac|SUM |
*********************************
|0  |0 | 00  |0|0 |0 |0 | 0|0x0 |
*********************************
|1  |1 | 00  |1|1 |0 |1 | 0|0x9A|
*********************************
|2  |1 | 00  |1|0 |0 |1 | 0|0x92|
*********************************
|3  |1 | 11  |1|1 |0 |1 | 0|0xFA|
*********************************
|4  |1 | 11  |1|0 |0 |1 | 0|0xF2|
*********************************
*/


#define SEG_DEFINED_FLAG 0xC //1100b => limit is in 4KiB block, 32 protected mode segment, not 64-bit code segment

#ifndef __ASSEMBLER__

/**
* @brief initialize a segement descriptor
*
* @param [out]desc segment descriptor
* @param [in]info
*/
static inline void seg_desc_init(struct seg_desc *desc, struct seg_desc_info *info)
{
    desc->limit1 = SEG_DESC_LIMIT1(info->limit);
    desc->limit2 = SEG_DESC_LIMIT2(info->limit);
    desc->base1 = SEG_DESC_BASE1(info->base);
    desc->base2 = SEG_DESC_BASE2(info->base);
    desc->base3 = SEG_DESC_BASE3(info->base);
    desc->access = info->access;
    desc->flags = SEC_DESC_FLAGS(info->flags);
}

struct x86_vmm;

/**
* @brief x86段初始化
*
* @param this 虚拟内存管理器
*
* @return TK_STATUS
*/
TK_STATUS seg_init(struct x86_vmm *this);

//设置tss, 这个只会在用户态想系统调用的时候用到,所以设置完一遍之后,不需要每次都重复设置了
void setup_tss(struct x86_vmm *this, u32 esp0);

#endif //__ASSEMBLER__

#endif
