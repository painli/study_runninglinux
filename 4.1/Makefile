#BASEINCLUDE ?= /lib/modules/`uname -r`/build
BASEINCLUDE ?= /home/pain/workplace/learn_kernel/kernel4.0 
# mytest为模块名   my_test为目标文件
mytest-objs := my_test.o
obj-m := mytest.o

# modinfo 命令可以查看module的信息
all:
	$(MAKE) -C $(BASEINCLUDE) M=$(PWD) modules;

clean:
	$(MAKE) -C $(BASEINCLUDE) SUBDIRS=$(PWD) clean;
	rm -f *.ko
