include $(ROOT_DIR)/Rule.mk

OBJS=x86_trap.o x86_vector.o x86_trap_asm.o x86_pic.o x86_drv.o

all: x86_kernel.ko

x86_kernel.ko: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS)
