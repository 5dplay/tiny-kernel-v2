include $(ROOT_DIR)/Rule.mk

OBJS=early_print.o

all: x86_lib.o

x86_lib.o: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS)
