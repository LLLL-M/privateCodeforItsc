ifneq ($(KERNELRELEASE),)
        obj-m := io.o
else
        #KERNELDIR ?= /lib/modules/$(shell uname -r)/build
        KERNELDIR ?= /opt/linux-at91-linux-2.6.39-at91-20160713
        PWD := $(shell pwd)
default: clean
	arm-none-linux-gnueabi-gcc io_test.c readline/libreadline.a -o io_test -Ireadline -lncurses
	make -C $(KERNELDIR) M=$(PWD) modules ARCH=arm CROSS_COMPILE=arm-none-linux-gnueabi-
	rm -rf *.o .*.cmd .tmp_versions  *.mod.c modules.order Module.symvers *.ko.unsigned
clean:
	$(RM) -r *.ko io_test *.o .*.cmd .tmp_versions  *.mod.c modules.order Module.symvers *.ko.unsigned
endif
