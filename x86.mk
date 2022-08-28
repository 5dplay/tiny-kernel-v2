ARCH_DIR=arch/x86

CFLAGS += -m32 -fno-pic -fno-omit-frame-pointer -fno-stack-protector -fno-pie
CFLAGS += -I$(ROOT_DIR)/$(ARCH_DIR)/include
LDFLAGS += -m elf_i386 -r
SUBDIRS += $(ARCH_DIR)
KERNEL_LDFLAGS = -m elf_i386 -T $(ARCH_DIR)/kernel.ld
MODULES = $(ARCH_DIR)/boot/boot.o $(ARCH_DIR)/lib/x86_lib.ko $(ARCH_DIR)/mm/x86_mm.ko $(ARCH_DIR)/kernel/x86_kernel.ko


QEMU = qemu-system-i386
QEMU_OPTS = -kernel image.elf -m 512

export MODULES
