include $(ROOT_DIR)/$(PRODUCT).conf

ifeq ($(CONFIG_EARLY_PRINT), y)
CFLAGS += -DEARLY_PRINT
endif

ifdef CONFIG_PAGE_SIZE
CFLAGS += -DPAGE_SIZE=$(CONFIG_PAGE_SIZE)
endif

%.o:%.c
	$(CC) -c $(CFLAGS) $< -o $@
%.o:%.S
	$(CC) $(CFLAGS) -c $< -o $@
