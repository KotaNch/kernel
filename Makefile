obj-m += main.o

KDIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(CURDIR)
CC ?= gcc

all:
	make -C $(KDIR) M=$(PWD) modules

tool: tool.c
	$(CC) -Wall -Wextra -O2 -o tool tool.c

user: tool

clean:
	make -C $(KDIR) M=$(PWD) clean
	rm -f tool
