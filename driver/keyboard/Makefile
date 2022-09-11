include $(ROOT_DIR)/Rule.mk

OBJS = keyboard.o

all: keyboard.ko

keyboard.ko: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS)
	cp $@ ../
