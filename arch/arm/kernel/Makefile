include $(ROOT_DIR)/Rule.mk

OBJS=arm_trap.o arm_vector.o arm_drv.o arm_proc.o switch_ctx.o user_init

all: arm_kernel.ko

arm_kernel.ko: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS)

user_init: user_init.o
	$(LD) $(LDFLAGS) -N -e start -Ttext 0 -o user_init.out user_init.o
	$(OBJCOPY) -S -O binary user_init.out user_init_code
	$(LD) $(LDFLAGS) -o user_init -b binary user_init_code
	$(OBJDUMP) -S user_init.o > $(@F).asm
