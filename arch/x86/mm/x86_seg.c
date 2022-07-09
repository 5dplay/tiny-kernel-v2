#include "common.h"
#include "string.h"
#include "x86_seg.h"
#include "x86_mm.h"
#include "x86_asm.h"

struct gdtr g_gdtr;
struct seg_desc_info g_seg_desc_info[SEG_MAX] = {
    {0x0, 0x0, 0x0}, //idx 0 not used
    {0x0, 0xFFFFF, 0x9A, 0xC},
    {0x0, 0xFFFFF, 0x92, 0xC},
    {0x0, 0xFFFFF, 0xFA, 0xC},
    {0x0, 0xFFFFF, 0xF2, 0xC},
    //slot for task state
};

int seg_init(struct x86_vmm *this)
{
    int i;
    memset(&this->m_seg, 0x0, sizeof(this->m_seg));

    this->m_ts.ss0 = SEG_KDATA << 3;
    this->m_ts.iomb = 0xFFFF;

    g_seg_desc_info[SEG_TSS].base = (uaddr)&this->m_ts;
    g_seg_desc_info[SEG_TSS].limit = sizeof(this->m_ts) - 1;
    g_seg_desc_info[SEG_TSS].access = 0x89;
    g_seg_desc_info[SEG_TSS].flags = 0x4;

    for (i = 0; i < SEG_TSS; i++)
        seg_desc_init(&this->m_seg[i], &g_seg_desc_info[i]);

    this->m_gdtr.size = sizeof(this->m_seg) - 1;
    this->m_gdtr.offset = (uaddr)&this->m_seg[0];

    //FIXME: 为何这里可以使用虚拟地址?
    lgdt((uaddr)&this->m_gdtr);
    reload_seg(SEG_KCODE << 3, SEG_KDATA << 3);

    return 0;
}

void setup_tss(struct x86_vmm *this, u32 esp0)
{
    seg_desc_init(&this->m_seg[SEG_TSS], &g_seg_desc_info[SEG_TSS]);

    this->m_ts.esp0 = esp0;
    ltr(SEG_TSS << 3);
}
