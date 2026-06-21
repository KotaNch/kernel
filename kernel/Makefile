obj-m += cyclic_buffer.o

KDIR := /lib/modules/$(shell uname -r)/build
PWD  := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

load:
	sudo insmod cyclic_buffer.ko

unload:
	sudo rmmod cyclic_buffer

reload: unload load

.PHONY: all clean load unload reload
