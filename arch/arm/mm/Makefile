include $(ROOT_DIR)/Rule.mk

OBJS=arm_mmu.o

all: arm_mm.ko

x86_mm.ko: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS)
