#ifndef __TINY_KERNEL_DRV_H__
#define __TINY_KERNEL_DRV_H__

#ifndef NO_DRV
void drv_init();
#else
static inline void drv_init()
{

}
#endif

#endif
