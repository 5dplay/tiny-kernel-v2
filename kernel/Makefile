include $(ROOT_DIR)/Rule.mk

ifeq ($(CONFIG_EARLY_PRINT), y)
CFLAGS += -DEARLY_PRINT
endif

OBJS=main.o

all: kernel.o

kernel.o: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS)
