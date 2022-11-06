include $(ROOT_DIR)/Rule.mk

OBJS = sd.o delays.o

all: emmc.ko

emmc.ko: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS)
	cp $@ ../
