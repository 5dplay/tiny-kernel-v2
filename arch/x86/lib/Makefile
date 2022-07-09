include $(ROOT_DIR)/Rule.mk

OBJS=early_print.o x86_string.o

all: x86_lib.ko

x86_lib.ko: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS)
