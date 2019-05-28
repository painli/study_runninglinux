BASEINCLUDE ?= /home/pain/code/kernel4.0
v_poll_s-objs := v_poll.o

obj-m = v_poll_s.o


all:
	$(MAKE) -C $(BASEINCLUDE) M=$(PWD) modules;

clean:
	$(MAKE) -C $(BASEINCLUDE) SUBDIRS=$(PWD) clean;
	@rm -f *.ko
