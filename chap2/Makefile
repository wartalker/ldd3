
ifneq ($(KERNELRELEASE),)
	obj-m := hello.o
else
	KERNELDIR ?= /lib/modules/3.11.6-200.fc19.x86_64/build/
	PWD := $(shell pwd)
default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
endif

clean:
	rm -f *.o *.ko *.mod.c modules.order Module.symvers
