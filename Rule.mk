include $(ROOT_DIR)/$(PRODUCT).conf

ifeq ($(CONFIG_EARLY_PRINT), y)
CFLAGS += -DEARLY_PRINT
endif

ifdef CONFIG_PAGE_SIZE
CFLAGS += -DPAGE_SIZE=$(CONFIG_PAGE_SIZE)
endif

ifeq ($(CONFIG_HAVE_ARCH_MEMSET), y)
CFLAGS += -DHAVE_ARCH_MEMSET
endif

ifeq ($(CONFIG_BOARD_RASPI2), y)
CFLAGS += -DBOARD_RASPI2
endif

%.o:%.c
	$(CC) -c $(CFLAGS) $< -o $@
%.o:%.S
	$(CC) $(CFLAGS) -c $< -o $@
