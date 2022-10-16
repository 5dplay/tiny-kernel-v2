ARCH_DIR=arch/arm

CFLAGS += -I$(ROOT_DIR)/$(ARCH_DIR)/include -march=armv7-a -mfpu=vfp -msoft-float -marm
LDFLAGS += -r
SUBDIRS += $(ARCH_DIR)
KERNEL_LDFLAGS = -T $(ARCH_DIR)/kernel.ld
MODULES = $(ARCH_DIR)/boot/boot.o $(ARCH_DIR)/lib/arm_lib.ko $(ARCH_DIR)/mm/arm_mm.ko $(ARCH_DIR)/kernel/arm_kernel.ko


QEMU = qemu-system-arm
QEMU_OPTS = -M raspi2b -kernel image -serial null -serial stdio 

export MODULES
