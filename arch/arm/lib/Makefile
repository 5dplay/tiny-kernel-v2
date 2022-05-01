include $(ROOT_DIR)/Rule.mk

OBJS=early_print.o uart.o

all: arm_lib.o

arm_lib.o: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS)
