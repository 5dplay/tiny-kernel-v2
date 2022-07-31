include $(ROOT_DIR)/Rule.mk

OBJS=math.o

all: lib.ko

lib.ko: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS)
