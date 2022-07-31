#ifndef __TINY_KERNEL_ARM_MM_H__
#define __TINY_KERNEL_ARM_MM_H__

#include "type.h"

struct arm_vmm {
    u32 reserved;   /* 目前不需要保存上下文内容，先占个坑 */
};

#endif
