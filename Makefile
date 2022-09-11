CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)ld
OBJCOPY = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump
CFLAGS = -std=gnu99 -O2 -ffreestanding -Wall -g
LDFLAGS =
SUBDIRS = kernel mm lib driver
ROOT_DIR = $(shell pwd)
MODULES =
DEF_MODULES = kernel/kernel.ko mm/mm.ko lib/lib.ko driver/drv.ko
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
	$(OBJDUMP) -S $(@).elf > $(@).asm

qemu: image
	$(QEMU) $(QEMU_OPTS)

qemu_debug: image
	$(QEMU) $(QEMU_OPTS) -S -s

qemu_gdb: image.elf
	gdb-multiarch image.elf --ex="target remote localhost:1234"

astyle:
	astyle --style=linux --recursive -s4 -S -H -p -U -f *.c *.h

clean:
	find . -name "*.o" | xargs rm -rf
	find . -name "*.ko" | xargs rm -rf
	find . -name "*.asm" | xargs rm -rf
	find . -name "*.orig" | xargs rm -rf
	rm -f image image.elf image.asm

.PHONY: clean $(patsubst %, _dir_%, $(SUBDIRS))
