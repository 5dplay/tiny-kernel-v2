arch = x86
ARCH_DIR=arch/x86

CFLAGS += -m32 -fno-pic -fno-omit-frame-pointer -fno-stack-protector -fno-pie  -fcf-protection=none -mmanual-endbr
CFLAGS += -I$(ROOT_DIR)/$(ARCH_DIR)/include
LDFLAGS += -m elf_i386 -r
USR_LDFLAGS = -m elf_i386
SUBDIRS += $(ARCH_DIR)
KERNEL_LDFLAGS = -m elf_i386 -T $(ARCH_DIR)/kernel.ld
MODULES = $(ARCH_DIR)/boot/boot.o $(ARCH_DIR)/lib/x86_lib.ko $(ARCH_DIR)/mm/x86_mm.ko $(ARCH_DIR)/kernel/x86_kernel.ko


QEMU = qemu-system-i386
QEMU_OPTS = -kernel image.elf -m 512 -hdb xv6.img

export MODULES
