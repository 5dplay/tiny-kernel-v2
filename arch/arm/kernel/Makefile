include $(ROOT_DIR)/Rule.mk

OBJS=arm_trap.o arm_vector.o arm_drv.o

all: arm_kernel.ko

arm_kernel.ko: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS)
