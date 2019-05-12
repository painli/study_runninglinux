#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>



#define DEMO_NAME "my_demo_dev"

static dev_t dev;
static struct cdev *demo_cdev;
static signed count = 1;




static int debug = 1;

#define dprintk(args...) \
	if(debug){ \
		printk(KERN_DEBUG args); \
       	}

static int demodrv_open(struct inode *inode, struct file *file){
	int major = MAJOR(inode->i_rdev);
	int minor = MINOR(inode->i_rdev);
	dprintk("%s: marjor=%d, minor = %d\n",__func__,major,minor);

	return 0;
}


static int demodrv_release(struct inode *inode, struct file *file){


	return 0;
}

static ssize_t demodrv_read(struct file *file, char __user *buf, size_t lbuf, loff_t *ppos){
	dprintk("%s enter\n",__func__);
	return 0;
}


static ssize_t demodrv_write(struct file *file, const char __user *buf, size_t lbuf, loff_t *ppos){

	dprintk("%s enter\n",__func__);
	return 0;
}

static const struct file_operations demodrv_fops = {
	.owner = THIS_MODULE,
	.open = demodrv_open,
	.release = demodrv_release,
	.read = demodrv_read,
	.write = demodrv_write
};

static int __init simple_char_init(void){
	int ret;
	/*
	 *  register_chrdev_region(dev_t first,unsigned int count,char *name)
	 *  First :要分配的设备编号范围的初始值, 这组连续设备号的起始设备号, 相当于register_chrdev（）中主设备号
	 *  Count:连续编号范围.   是这组设备号的大小（也是次设备号的个数）
	 *  Name:编号相关联的设备名称. (/proc/devices); 本组设备的驱动名称
	 *
	 *  alloc_chrdev_region函数，来让内核自动给我们分配设备号
	 *  (1)register_chrdev_region是在事先知道要使用的主、次设备号时使用的；要先查看cat /proc/devices去查看没有使用的。
	 *  (2)更简便、更智能的方法是让内核给我们自动分配一个主设备号，使用alloc_chrdev_region就可以自动分配了。
	 *  (3)自动分配的设备号，我们必须去知道他的主次设备号，否则后面没法去mknod创建他对应的设备文件
	 *
	 *  int alloc_chrdev_region(dev_t *dev, unsigned baseminor, unsigned count, const char *name)
	 *  dev 分配得到的设备号，可以用MAJOR宏和MINOR宏，将主设备号和次设备号
	 *  baseminor: 次设备号的基准，从第几个次设备号开始分配
	 *  count: 次设备号的个数
	 *  name: 驱动的名字
	 */
	ret = alloc_chrdev_region(&dev,0,count,DEMO_NAME);
	if(ret){
		dprintk("failed to allocate char device region");
		return ret;
	}

	/*
	 * cdev_alloc：让内核为这个结构体分配内存的
	 * cdev_init：将struct cdev类型的结构体变量和file_operations结构体进行绑定的
	 * cdev_add：向内核里面添加一个驱动，注册驱动
	 * cdev_del：从内核中注销掉一个驱动。注销驱动
	 */
	demo_cdev = cdev_alloc();
	if(!demo_cdev){
		dprintk("cdev_alloc failed");
		goto unregister_chrdev;
	}
	cdev_init(demo_cdev,&demodrv_fops);


	ret = cdev_add(demo_cdev,dev,count);
	if(ret){
		dprintk("cdev_add failed");
		goto cdev_fail;
	}
	dprintk("succeed register char device:%s\n",DEMO_NAME);
	dprintk("Major number = %d, minor number = %d\n",
			MAJOR(dev),MINOR(dev));
	return 0;
cdev_fail:
	cdev_del(demo_cdev);
unregister_chrdev:
	unregister_chrdev_region(dev,count);

	return 0;
}


static void __exit simple_char_exit(void){

}


module_init(simple_char_init);
module_exit(simple_char_exit);


MODULE_AUTHOR("pain.li");
MODULE_LICENSE("GPL V2");
MODULE_DESCRIPTION("simple character device");
