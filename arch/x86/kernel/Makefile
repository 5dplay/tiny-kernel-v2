include $(ROOT_DIR)/Rule.mk

OBJS=x86_trap.o x86_vector.o x86_trap_asm.o x86_pic.o x86_drv.o switch_ctx.o x86_proc.o user_init

all: x86_kernel.ko

x86_kernel.ko: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS)

user_init: user_init.o
	$(LD) $(LDFLAGS) -N -e start -Ttext 0 -o user_init.out user_init.o
	$(OBJCOPY) -S -O binary user_init.out user_init_code
	$(LD) $(LDFLAGS) -o user_init -b binary user_init_code
	$(OBJDUMP) -S user_init.o > $(@F).asm