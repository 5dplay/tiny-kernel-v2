include $(ROOT_DIR)/Rule.mk

OBJS=main.o string.o printk.o proc.o bio.o block_dev.o exec.o sys_call.o sys_file.o console.o

all: kernel.ko

kernel.ko: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS)
