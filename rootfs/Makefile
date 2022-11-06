CFLAGS += -I./ -D$(ARCH)
LDFLAGS = $(USR_LDFLAGS)
include $(ROOT_DIR)/Rule.mk

ULIB = libc/$(ARCH)_usys.o libc/printf.o libc/ulib.o
ifeq ($(ARCH),arm)
ULIB += libc/arm_lib.o
endif
BIN = init sh ls sleep
all: $(BIN)

init: tinybox/init.o $(ULIB)
	$(LD) $(LDFLAGS) -N -e main -Ttext 0 -o $@ $^
	$(OBJDUMP) -d $@ > $@.asm

sh: tinybox/sh.o $(ULIB)
	$(LD) $(LDFLAGS) -N -e main -Ttext 0 -o $@ $^
	$(OBJDUMP) -d $@ > $@.asm

ls: tinybox/ls.o $(ULIB)
	$(LD) $(LDFLAGS) -N -e main -Ttext 0 -o $@ $^
	$(OBJDUMP) -d $@ > $@.asm

sleep: tinybox/sleep.o $(ULIB)
	$(LD) $(LDFLAGS) -N -e main -Ttext 0 -o $@ $^
	$(OBJDUMP) -d $@ > $@.asm

clean:
	rm -f $(BIN)

.PHONY: clean
