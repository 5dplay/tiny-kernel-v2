#ifndef __TINY_KERNEL_X86_MM_H__
#define __TINY_KERNEL_X86_MM_H__

#include "mm.h"
#include "x86_seg.h"

struct x86_vmm {
    struct seg_desc m_seg[SEG_MAX];     /* 段描述符 */
    struct gdtr m_gdtr;                 /* 全局描述符表 */
    struct task_state m_ts;             /* tss */
};

#endif
