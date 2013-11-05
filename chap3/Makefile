
version=$(shell uname -r)-r1
#version="3.11.6-200.fc19.x86_64"

ifneq ($(KERNELRELEASE),)
	obj-m := scull.o
else
	KERNELDIR ?= /lib/modules/$(version)/build/
	PWD := $(shell pwd)
default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
endif

clean:
	rm -f *.o *.ko *.mod.c modules.order Module.symvers
