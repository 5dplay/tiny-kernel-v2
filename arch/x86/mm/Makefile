include $(ROOT_DIR)/Rule.mk

OBJS=x86_mmu.o x86_seg.o x86_page.o

all: x86_mm.ko

x86_mm.ko: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS)
