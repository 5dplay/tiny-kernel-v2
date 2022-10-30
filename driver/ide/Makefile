include $(ROOT_DIR)/Rule.mk

OBJS = ide.o

all: ide.ko

ide.ko: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS)
	cp $@ ../
