ifneq ($(KERNELRELEASE),)
	obj-m := globalvar.o
else
	KERNELDIR := /lib/modules/$(shell uname -r)/build #定义一个变量,指向内核目录
	PWD := $(shell pwd)
modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
endif
clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean