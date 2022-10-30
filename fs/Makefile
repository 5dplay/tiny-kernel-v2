include $(ROOT_DIR)/Rule.mk

OBJS=vfs.o indexfs.o tinyfs.o

all: fs.ko

fs.ko: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS)
