BASEINCLUDE ?= /home/pain/workplace/learn_kernel/kernel4.0

mydemo-objs := my_demodev.o

obj-m := mydemo.o


all:
	$(MAKE) -C $(BASEINCLUDE) M=$(PWD) modules;

clean:
	$(MAKE) -C $(BASEINCLUDE) SUBDIRS=$(PWD) clean;
	rm -f *.ko
