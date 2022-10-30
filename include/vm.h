#ifndef __TINY_KERNEL_VM_H__
#define __TINY_KERNEL_VM_H__

#include "type.h"

struct virtual_memory_managment;
typedef struct virtual_memory_managment vmm;

enum vm_perm_flags_e {
    VM_PERM_WRITE = 0x1,    /* 默认是可读的,标记是否可写即可 */
    VM_PERM_USER = 0x2,     /* 默认内核可访问,标记用户是否可访问即可 */
};

struct inode;
struct vm_prog_load_params {
    uaddr pg_dir;
    uaddr usr_va;
    struct inode *ip;
    uint off;
    uint size;
};

/* FIXME: 思考是否需要vmm *vm参数，看起来没什么必要性 */

/**
* @brief 分配一个页目录，用于后续的虚拟内存映射
*
* @param [in]vm 虚拟内存管理器
*
* @return 页目录
*/
uaddr vm_alloc_pg_dir(vmm *vm);

/**
* @brief 拷贝一份用户进程的虚拟地址映射到新的页目录中
*
* @param [in]vm 虚拟内存管理器
* @param [in]dst_pg_dir 目标页目录
* @param [in]src_pg_dir 源页目录
* @param [in]va_end 虚拟内存地址结束处，默认从0开始，也就是拷贝[0, va_end)
*
* @return TK_STATUS
*/
TK_STATUS vm_clone_pg_dir(vmm *vm, uaddr dst_pg_dir, uaddr src_pg_dir, uaddr va_end);

/**
* @brief 回收页目录，解除虚拟内存地址映射关系
*
* @param [in]vm 虚拟内存管理器
* @param [in]pg_dir 页目录
*/
void vm_free_pg_dir(vmm *vm, uaddr pg_dir);

/**
* @brief 虚拟内存地址映射
*
* @param [in]vm 虚拟内存管理器
* @param [in[pg_dir 页目录
* @param [in]v_addr 虚拟内存地址
* @param [in]p_addr 对应的物理地址
* @param [in]size 申请映射内存块大小
* @param [in]perm_flags 权限，见vm_perm_flags_e
*
* @return TK_STATUS
*/
TK_STATUS vm_map(vmm *vm, uaddr pg_dir, uaddr v_addr, uaddr p_addr,
                 uint size, uint perm_flags);

/**
* @brief 重新调整用户虚拟内存大小[0, old_sz) -> [0, new_sz)
*
* @param [in]vm 虚拟内存管理器
* @param [in]pg_dir 页目录
* @param [in]new_sz 新的虚拟内存结束地址
* @param [in]old_sz 旧的虚拟内存结束地址
*
* @return 调整后的大小
*/
uint vm_resize(vmm *vm, uaddr pg_dir, uint old_sz, uint new_sz);

TK_STATUS vm_prog_load(vmm *vm, struct vm_prog_load_params *params);

/**
* @brief 重新加载内存地址
*
* @param [in]vm 虚拟内存管理器
* @param [in]pg_dir 页目录
*/
void vm_reload(vmm *vm, uaddr pg_dir);

/**
* @brief 各架构内存管理初始化
*
* @return NULL => fail, not NULL => 虚拟内存管理器
*/
vmm *arch_mmu_init();

/**
* @brief 获取虚拟内存管理器
*
* @return 失败 => NULL
*/
vmm *get_mmu();

TK_STATUS kmap(uaddr v_addr, uaddr p_addr, uint size, uint perm_flags);
TK_STATUS kernel_map_init(vmm *p_vm, uaddr pg_dir);
void switch_kvm();

/* defined in each arch */
struct proc;
void switch_uvm(struct proc *p);

#endif
