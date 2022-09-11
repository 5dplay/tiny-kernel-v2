include $(ROOT_DIR)/Rule.mk

drv-dir-${CONFIG_KEYBOARD} = keyboard
drv-dir-${CONFIG_MINI_UART} = mini_uart

OBJS = $(patsubst %, %.ko, $(drv-dir-y))
$(info $(drv-dir-y))
all: drv.ko

drv.ko: $(patsubst %, _dir_%, $(drv-dir-y))
	$(LD) $(LDFLAGS) -o $@ $(OBJS)

$(patsubst %, _dir_%, $(drv-dir-y)):
	$(MAKE) -C $(patsubst _dir_%, %, $@)

.PHONY: dummy $(patsubst %, _dir_%, $(drv-dir-y))
