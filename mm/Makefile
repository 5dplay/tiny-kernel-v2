include $(ROOT_DIR)/Rule.mk

OBJS=page_alloc.o bootm_alloc.o mm.o

all: mm.ko

mm.ko: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS)
