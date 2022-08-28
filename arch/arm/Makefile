SUBDIRS = boot lib mm kernel

all: $(patsubst %, _dir_%, $(SUBDIRS))

$(patsubst %, _dir_%, $(SUBDIRS)):
	$(MAKE) -C $(patsubst _dir_%, %, $@)

.PHONY: dummy $(patsubst %, _dir_%, $(SUBDIRS))
