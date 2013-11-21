obj-m := ProcRW.o
KERNELDIR := /lib64/modules/$(shell uname -r)/build
PWD := $(shell pwd)

default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

clean:
	rm -rf *.o *~ core .depend *.order *.symvers .*.cmd *.ko *.mod.c .tmp_versions
