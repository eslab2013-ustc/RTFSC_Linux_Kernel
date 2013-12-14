obj-m := SeqFile.o
KERNELDIR := /lib64/modules/$(shell uname -r)/build
PWD := $(shell pwd)

default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

clean:
	rm -rf *.o *~ core .depend *.order *.symvers .*.cmd *.ko *.mod.c .tmp_versions

ins:
	insmod SeqFile.ko

rm:
	rmmod SeqFile
