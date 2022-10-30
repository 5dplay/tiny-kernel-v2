#include "common.h"
#include "limits.h"
#include "memlayout.h"
#include "mm.h"
#include "page.h"
#include "proc.h"
#include "vfs.h"
#include "indexfs.h"
#include "sys_call.h"
#include "elf.h"
#include "string.h"
#include "vm.h"

static int do_exec(char *path, char **argv);

int sys_test(void)
{
    char *fmt;
    fmt = NULL;
    arg_ptr(0, (void **)&fmt);

    if (fmt)
        printk(fmt);

    printk("\n");
    return 0;
}

int sys_exec(void)
{
    char *path, *argv[MAXARG];
    uaddr *uargv;
    uint i;

    //获取执行路径,以及可变参数起始位置
    arg_ptr(0, (void **)&path);
    arg_ptr(1, (void **)&uargv);

    i = 0;

    while (i < MAXARG) {
        argv[i] = (char *)uargv[i];

        if (argv[i] == NULL)
            break;

        i++;
    }

    return do_exec(path, argv);
}

static int do_exec(char *path, char **argv)
{
    struct inode *ip;
    vmm *vm;
    uint cnt, i, off, sz;
    int pg_dir, old_pg_dir;
    struct elfhdr elf;
    struct vm_prog_load_params params;
    struct proghdr ph;
    struct proc *p;
    u8 *mem1, *mem2;
    char *last, *s;
    uint ustack[3 + MAXARG + 1], sp;

    p = get_cur_proc();
    vm = get_mmu();
    ip = namei(path);

    if (ip == NULL) {
        printk("%s: %s not found.\n", __func__, path);
        return -1;
    }

    cnt = data_readi(ip->superb, ip, (u8 *)&elf, 0, sizeof(elf));

    //FIXME: 这里一概不考虑大小端的问题, 等一切弄完之后再来处理该问题
    if (cnt != sizeof(elf) || elf.magic != ELF_MAGIC)
        goto release_inode;

    pg_dir = vm_alloc_pg_dir(vm);
    kernel_map_init(vm, pg_dir);

    sz = 0;
    params.pg_dir = pg_dir;
    params.ip = ip;

    for (i = 0, off = elf.phoff; i < elf.phnum; i++, off += sizeof(ph)) {
        cnt = data_readi(ip->superb, ip, (u8 *)&ph, off, sizeof(ph));

        if (cnt != sizeof(ph))
            goto free_vm;

        if (ph.type != ELF_PROG_LOAD)
            continue;

        if (ph.memsz < ph.filesz)
            goto free_vm;

        if (ph.vaddr + ph.memsz < ph.vaddr)
            goto free_vm;

        if (ph.vaddr + ph.memsz > sz)
            sz = vm_resize(vm, pg_dir, sz, ph.vaddr + ph.memsz);

        if (sz == 0)
            goto free_vm;

        params.usr_va = ph.vaddr;
        params.off = ph.off;
        params.size = ph.filesz;

        if (vm_prog_load(vm, &params) < 0)
            goto free_vm;
    }

    sz = PAGE_ALIGN_CEIL(sz);
    //FIXME: 异常处理
    mem1 = page_alloc();
    mem2 = page_alloc();

    vm_map(vm, pg_dir, sz, virt_to_phy((uaddr)mem1), PAGE_SIZE, VM_PERM_WRITE);

    vm_map(vm, pg_dir, sz + PAGE_SIZE, virt_to_phy((uaddr)mem2), PAGE_SIZE,
           VM_PERM_WRITE | VM_PERM_USER);

    //TODO: 这里有可能需要做平台差异化,目前假定都是32位,并且栈向下
    sz += 2 * PAGE_SIZE;
    sp = sz;
    //先将mem2挪到最后
    mem2 += PAGE_SIZE;

    //先拷贝所有字符串,然后再重新打上相关的字符串指针
    for (i = 0; argv[i]; i++) {
        off = ((strlen(argv[i]) + 1) + 0x3) & (~0x3); //ROUNDUP
        mem2 -= off;
        sp -= off;
        memcpy(mem2, argv[i], strlen(argv[i]) + 1);
        ustack[3 + i] = sp;
    }

    ustack[3 + i] = 0;

    ustack[0] = 0xffffffff; //fake return pc
    ustack[1] = i; //argc
    ustack[2] = sp - (i + 1) * 4; //argv point to ustack[3] vaddr
    off = (3 + i + 1) * 4;
    mem2 -= off;
    sp -= off;
    memcpy(mem2, ustack, off);

    for (last = s = path; *s; s++)
        if (*s == '/')
            last = s + 1;

    strncpy_s(p->comm, last, sizeof(p->comm));
    old_pg_dir = p->pg_dir;
    p->pg_dir = pg_dir;
    p->mem_size = sz;
    usr_exec_proc(p, elf.entry, sp);

    switch_uvm(p);
    vm_free_pg_dir(vm, old_pg_dir);
    printk("%s: leave\n", __func__);
    return 0;
//FIXME: 完善相关的内容
free_vm:

    if (pg_dir)
        vm_free_pg_dir(vm, pg_dir);

release_inode:
    return -1;
}
