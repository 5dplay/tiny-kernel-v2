include $(ROOT_DIR)/$(PRODUCT).conf

%.o:%.c
	$(CC) -c $(CFLAGS) $< -o $@
%.o:%.S
	$(CC) $(CFLAGS) -c $< -o $@
