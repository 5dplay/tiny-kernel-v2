include $(ROOT_DIR)/Rule.mk


OBJS=main.o

all: kernel.ko

kernel.ko: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS)
