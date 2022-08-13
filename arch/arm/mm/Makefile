include $(ROOT_DIR)/Rule.mk

OBJS=arm_mmu.o arm_page.o

all: arm_mm.ko

arm_mm.ko: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS)
