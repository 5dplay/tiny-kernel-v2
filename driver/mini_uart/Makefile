include $(ROOT_DIR)/Rule.mk

OBJS = mini_uart.o

all: mini_uart.ko

mini_uart.ko: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS)
	cp $@ ../
