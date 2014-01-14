obj-m := my_seq_file.o
KERNELDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
clean:
	rm -rf *.o *~ core .depend *.order *.symvers .*.cmd *.ko *.mod.c .tmp_versions
