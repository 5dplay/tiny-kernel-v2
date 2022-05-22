include $(ROOT_DIR)/Rule.mk

OBJS=uart.o

ifeq ($(CONFIG_EARLY_PRINT), y)
OBJS += early_print.o
endif

all: arm_lib.o

arm_lib.o: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS)
