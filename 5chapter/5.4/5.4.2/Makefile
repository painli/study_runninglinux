#BASEINCLUDE ?= /home/pain/workplace/learn_kernel/kernel4.0
BASEINCLUDE ?= /home/pain/code/kernel4.0 

mydemo_misc_v_fifo-objs := my_demodev.o

obj-m := mydemo_misc_v_fifo.o



all:
	$(MAKE) -C $(BASEINCLUDE) M=$(PWD) modules;

clean:
	$(MAKE) -C $(BASEINCLUDE) SUBDIRS=$(PWD) clean;
	@rm -f *.ko
