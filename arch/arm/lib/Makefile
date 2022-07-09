include $(ROOT_DIR)/Rule.mk

OBJS=uart.o

ifeq ($(CONFIG_EARLY_PRINT), y)
OBJS += early_print.o
endif

all: arm_lib.ko

arm_lib.ko: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS)
