include $(ROOT_DIR)/Rule.mk

OBJS=page_alloc.o bootm_alloc.o

all: mm.o

mm.o: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS)
