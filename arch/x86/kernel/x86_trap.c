#include "common.h"
#include "x86_trap.h"
#include "x86_asm.h"

struct idt_gate_desc g_idt[IDT_ENTRY_MAX];
struct idtr g_idt_reg;
trap_handler g_trap_handler[IDT_ENTRY_MAX];

extern uaddr vectors[];
static void exception_init();

TK_STATUS trap_init()
{
    int i;

    for (i = 0; i < IDT_ENTRY_BUILTIN; i++)
        set_trap_gate(&g_idt[i], vectors[i]);

    set_system_gate(&g_idt[T_BRKPT], vectors[T_BRKPT]);
    set_system_gate(&g_idt[T_OFLOW], vectors[T_OFLOW]);
    set_system_gate(&g_idt[T_BOUND], vectors[T_BOUND]);

    for (; i < IDT_ENTRY_MAX; i++)
        set_intr_gate(&g_idt[i], vectors[i]);

    set_system_gate(&g_idt[T_SYSCALL], vectors[T_SYSCALL]);

    g_idt_reg.size = sizeof(g_idt) - 1;
    g_idt_reg.offset = (uaddr)&g_idt[0];
    lidt((uaddr)&g_idt_reg);

    exception_init();

    return TK_STATUS_SUCCESS;
}

static int trap_reg(int trap_no, trap_handler func)
{
    if (trap_no < 0 || trap_no >= IDT_ENTRY_MAX) {
        printk("trap range [0, %d)", IDT_ENTRY_MAX);
        return -1;
    }

    if (NULL != g_trap_handler[trap_no]) {
        printk("trap %x was already registered\n", trap_no);
        return -1;
    }

    g_trap_handler[trap_no] = func;
    return 0;
}

#if 0
static int trap_unreg(int trap_no, trap_handler func)
{
    if (trap_no < 0 || trap_no >= IDT_ENTRY_MAX) {
        printk("trap_no %d out of range [0, %d)", trap_no, IDT_ENTRY_MAX);
        return -1;
    }

    g_trap_handler[trap_no] = NULL;
    return 0;
}
#endif

static void show_tf(struct trap_frame *tf)
{
    printk("eip: 0x%08x eflags: 0x%08x\n", tf->eip, tf->eflags);
    printk("esp: 0x%08x ebp: 0x%08x esi: 0x%08x edi: 0x%08x\n",
           tf->ori_esp, tf->ebp, tf->esi, tf->edi);
    printk("eax: 0x%08x ebx: 0x%08x ecx: 0x%08x edx: 0x%08x\n",
           tf->eax, tf->ebx, tf->ecx, tf->edx);
}
#define DECLARE_TRAP_HANDLER(func_name)                             \
static void func_name(struct trap_frame *frame)                     \
{                                                                   \
    show_tf(frame);                                                 \
    panic(#func_name" trigger, irqno == %d, err_code = %d!\n",      \
         frame->trap_no, frame->error_code);                        \
}

DECLARE_TRAP_HANDLER(trap_div_err);
DECLARE_TRAP_HANDLER(trap_debug);
DECLARE_TRAP_HANDLER(trap_nmi);
DECLARE_TRAP_HANDLER(trap_int3);
DECLARE_TRAP_HANDLER(trap_overflow);
DECLARE_TRAP_HANDLER(trap_bounds);
DECLARE_TRAP_HANDLER(trap_invalid_op);
DECLARE_TRAP_HANDLER(trap_invalid_device);
DECLARE_TRAP_HANDLER(trap_double_fault);
DECLARE_TRAP_HANDLER(trap_coprocessor_segment_overrun);
DECLARE_TRAP_HANDLER(trap_invalid_tss);
DECLARE_TRAP_HANDLER(trap_segment_not_present);
DECLARE_TRAP_HANDLER(trap_stack_seg);
DECLARE_TRAP_HANDLER(trap_general_protection);
DECLARE_TRAP_HANDLER(trap_reserved);
DECLARE_TRAP_HANDLER(trap_coprocessor_err);
DECLARE_TRAP_HANDLER(trap_alignment_check);

//extern void trap_syscall(struct trap_frame *frame);
void trap_page_fault(struct trap_frame *frame)
{
    uintptr_t addr;
    __asm__("movl %%cr2, %0" : "=r"(addr));
    show_tf(frame);
    panic("dp_pg_fault, addr=0x%p, err_code=%d\n", addr, frame->error_code);
}

static void exception_init()
{
    int i = 0;
    trap_reg(i++, trap_div_err);
    trap_reg(i++, trap_debug);
    trap_reg(i++, trap_nmi);
    trap_reg(i++, trap_int3);
    trap_reg(i++, trap_overflow);
    trap_reg(i++, trap_bounds);
    trap_reg(i++, trap_invalid_op);
    trap_reg(i++, trap_invalid_device);
    trap_reg(i++, trap_double_fault);
    trap_reg(i++, trap_coprocessor_segment_overrun);
    trap_reg(i++, trap_invalid_tss);
    trap_reg(i++, trap_segment_not_present);
    trap_reg(i++, trap_stack_seg);
    trap_reg(i++, trap_general_protection);
    trap_reg(i++, trap_page_fault);
    trap_reg(i++, trap_reserved);
    trap_reg(i++, trap_coprocessor_err);
    trap_reg(i++, trap_alignment_check);

    for (; i < IDT_ENTRY_BUILTIN; i++)
        trap_reg(i, trap_reserved);

#ifdef SUPPORT_SYSCALL
    void trap_syscall(struct trap_frame * tf);
    trap_reg(T_SYSCALL, trap_syscall);
#endif
}

/* FIXME: 以下这两个函数暂时放在这里，后续处理arm的时候看一下如何抽象出来 */
static int get_trap_no(struct trap_frame * tf)
{
    return tf->trap_no;
}

void trap_svr(struct trap_frame *tf)
{
    int trap_no;

    trap_no = get_trap_no(tf);

    if (0 < trap_no && trap_no < IDT_ENTRY_MAX && g_trap_handler[trap_no])
        g_trap_handler[trap_no](tf);
}

