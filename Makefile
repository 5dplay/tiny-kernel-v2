CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)ld
OBJCOPY = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump
CFLAGS = -std=gnu99 -O2 -ffreestanding -Wall
LDFLAGS =
SUBDIRS = kernel mm lib fs driver
ROOT_DIR = $(shell pwd)
MODULES =
DEF_MODULES = kernel/kernel.ko mm/mm.ko lib/lib.ko fs/fs.ko driver/drv.ko
ARCH_DIR = 
CFLAGS += -I$(ROOT_DIR)/include

QEMU =
QEMU_OPTS =

ifeq ($(PRODUCT),)
PRODUCT = x86
export PRODUCT
endif

include $(PRODUCT).mk

export CC LD OBJCOPY OBJDUMP CFLAGS LDFLAGS DEF_MODULES ROOT_DIR

all: subdirs image

subdirs: $(patsubst %, _dir_%, $(SUBDIRS))

$(patsubst %, _dir_%, $(SUBDIRS)):
	$(MAKE) -C  $(patsubst _dir_%, %, $@)

image: subdirs
	$(LD) $(KERNEL_LDFLAGS) $(DEF_MODULES) $(MODULES) -o $(@).elf
	$(OBJCOPY) -O binary $(@).elf $@
	$(OBJDUMP) -d $(@).elf > $(@).asm

qemu: image
	$(QEMU) $(QEMU_OPTS)

qemu_debug: image
	$(QEMU) $(QEMU_OPTS) -S -s

qemu_gdb: image.elf
	gdb-multiarch image.elf --ex="target remote localhost:1234"

xv6.img:
	$(MAKE) -C mkfs.tinyfs
	$(MAKE) -C rootfs
	cp rootfs/init mkfs.tinyfs/rootfs/
	cp rootfs/sh mkfs.tinyfs/rootfs/
	cp rootfs/ls mkfs.tinyfs/rootfs/
	cp rootfs/sleep mkfs.tinyfs/rootfs/
	./mkfs.tinyfs/mkfs -d ./mkfs.tinyfs/rootfs -o $(@)

astyle:
	astyle --style=linux --recursive -s4 -S -H -p -U -f *.c *.h

clean:
	find . -name "*.o" | xargs rm -rf
	find . -name "*.ko" | xargs rm -rf
	find . -name "*.asm" | xargs rm -rf
	find . -name "*.orig" | xargs rm -rf
	rm -f image image.elf image.asm xv6.img
	$(MAKE) -C mkfs.tinyfs clean
	$(MAKE) -C rootfs clean

.PHONY: clean xv6.img $(patsubst %, _dir_%, $(SUBDIRS))
