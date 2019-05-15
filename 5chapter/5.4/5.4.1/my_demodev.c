#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/miscdevice.h>


#define DEMO_NAME "my_demo_dev"


#define MAX_DEVICE_BUFFER_SIZE 64
static char* device_buffer;



static struct device *mydemodrv_device;
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
	int actual_read;
	int max_free;
	int need_read;
	int ret;
	
	max_free = MAX_DEVICE_BUFFER_SIZE - *ppos;
	need_read = max_free > lbuf ? lbuf : max_free;

	ret = copy_to_user(buf,device_buffer + *ppos,need_read);

	if(ret == need_read){
		return -EFAULT;
	}
	actual_read = need_read - ret;
	*ppos += actual_read;

	dprintk("%s: actual_read = %d, pos = %d\n",__func__,actual_read,*ppos);
	return actual_read;
}


static ssize_t demodrv_write(struct file *file, const char __user *buf, size_t lbuf, loff_t *ppos){

	dprintk("%s enter\n",__func__);
	int free;
	int need_write;
	int actual_write;
	int ret;


	free = MAX_DEVICE_BUFFER_SIZE - *ppos;
	need_write = free > lbuf ? lbuf : free;
	if(0 == need_write){
		dev_warn(mydemodrv_device,"no space for write");
	}

	ret = copy_from_user(device_buffer + *ppos,buf,need_write);
	if(ret == need_write){
		return -EFAULT;
	}
	actual_write = need_write - ret;
	*ppos += actual_write;
	printk("%s:actual_write = %d, ppos=%d\n",__func__,actual_write,*ppos);
	return actual_write;

	return 0;
}

static const struct file_operations demodrv_fops = {
	.owner = THIS_MODULE,
	.open = demodrv_open,
	.release = demodrv_release,
	.read = demodrv_read,
	.write = demodrv_write
};

static struct miscdevice mydemodrv_misc_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = DEMO_NAME,
	.fops = &demodrv_fops,
};

static int __init simple_char_init(void){
	int ret;
	ret = misc_register(&mydemodrv_misc_device);
	if(ret){
		printk("failed register misc device\n");
		return ret;
	}

	mydemodrv_device = mydemodrv_misc_device.this_device;
	printk("succeeded register char device: %s\n",DEMO_NAME);
	return 0;
}


static void __exit simple_char_exit(void){
	printk("removing device\n");
	misc_deregister(&mydemodrv_misc_device);
}


module_init(simple_char_init);
module_exit(simple_char_exit);


MODULE_AUTHOR("pain.li");
MODULE_LICENSE("GPL V2");
MODULE_DESCRIPTION("simple character device");
