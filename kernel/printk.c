#include <stdarg.h>
#include "common.h"
#include "string.h"

static char printk_buf[1024];
static void (*console_print_proc)(const char *msg) = early_print;

int printk(const char *fmt, ...)
{
    va_list args;
    int i;
    //long flags;

    //TODO: 补全中断相关的
    //save_flags(flags); //保存FLAGS
    //cli(); //禁用中断

    //获取可变参数列表的第一个参数的地址（args是类型为va_list的指针，fmt是可变参数最左边的参数）
    //这样子才可以确定最后参数的总大小,如果是从左到右的话,那么知道第一个参数的地址并没有什么用. 这些都是约定俗成的东西,大可以约定必须给出最右边的参数名字,那这样就可以从左到右压栈了
    va_start(args, fmt);
    i = vsprintf(printk_buf, fmt, args);
    va_end(args);

    if (console_print_proc)
        console_print_proc(printk_buf);

    //restore_flags(flags)
    return i;
}

void panic(const char *fmt, ...)
{
    va_list args;
    //long flags;

    //TODO: 补全中断相关的
    //save_flags(flags); //保存FLAGS
    //cli(); //禁用中断

    //获取可变参数列表的第一个参数的地址（args是类型为va_list的指针，fmt是可变参数最左边的参数）
    va_start(args, fmt);
    vsprintf(printk_buf, fmt, args);
    va_end(args);

    if (console_print_proc)
        console_print_proc(printk_buf);

    //restore_flags(flags)
    while (1) ;
}

void register_console(void (*proc)(const char *msg))
{
    console_print_proc = proc;
}
