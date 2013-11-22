
ifneq ($(KERNELRELEASE),)
	obj-m := ldd_bus.o
else
	KERNELDIR ?= /lib/modules/$(shell uname -r)/build/
	PWD := $(shell pwd)
default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
endif

clean:
	rm -rf *.o *.ko *.mod.c modules.order Module.symvers .*.cmd .tmp_versions
