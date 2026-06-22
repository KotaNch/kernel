all:
	$(MAKE) -C kernel
	$(MAKE) -C userspace

clean:
	$(MAKE) -C kernel clean
	$(MAKE) -C userspace clean

load:
	$(MAKE) -C kernel load

unload:
	$(MAKE) -C kernel unload

.PHONY: all clean load unload