ARCH_DIR=arch/arm

CFLAGS += -I$(ROOT_DIR)/$(ARCH_DIR)
LDFLAGS += -r
SUBDIRS += $(ARCH_DIR)
KERNEL_LDFLAGS = -T $(ARCH_DIR)/kernel.ld
MODULES = $(ARCH_DIR)/boot/boot.o $(ARCH_DIR)/lib/arm_lib.o

QEMU = qemu-system-aarch64
QEMU_OPTS = -M raspi2b -kernel image -serial null -serial stdio 

export MODULES
