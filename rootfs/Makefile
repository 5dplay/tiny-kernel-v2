CFLAGS += -I./
LDFLAGS = -m elf_i386
include $(ROOT_DIR)/Rule.mk

ULIB = libc/usys.o libc/printf.o libc/ulib.o
BIN = init sh ls sleep
all: $(BIN)

init: tinybox/init.o $(ULIB)
	$(LD) $(LDFLAGS) -N -e main -Ttext 0 -o $@ $^
	$(OBJDUMP) -S $@ > $@.asm

sh: tinybox/sh.o $(ULIB)
	$(LD) $(LDFLAGS) -N -e main -Ttext 0 -o $@ $^
	$(OBJDUMP) -S $@ > $@.asm

ls: tinybox/ls.o $(ULIB)
	$(LD) $(LDFLAGS) -N -e main -Ttext 0 -o $@ $^
	$(OBJDUMP) -S $@ > $@.asm

sleep: tinybox/sleep.o $(ULIB)
	$(LD) $(LDFLAGS) -N -e main -Ttext 0 -o $@ $^
	$(OBJDUMP) -S $@ > $@.asm

clean:
	rm -f $(BIN)

.PHONY: clean
