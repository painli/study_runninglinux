#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/kfifo.h>

DEFINE_KFIFO(mydemo_fifo,char,64);
#define DEMO_NAME "my_demo_dev"


#define MAX_DEVICE_BUFFER_SIZE 64

struct mydemo_device{
        const char *name;
        struct device *dev;
        wait_queue_head_t read_queue;
        wait_queue_head_t write_queue;
};
static struct mydemo_device device = {
        .name = DEMO_NAME,
};

//static struct device *mydemodrv_device;
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
	int ret;
	
    if(kfifo_is_empty(&mydemo_fifo)){
            if(file->f_flags & O_NONBLOCK)
            {
                return -EAGAIN;
            }
            dprintk("%s :pid=%d,going to sleep\n",__func__,current->pid);
            ret = wait_event_interruptible(device.read_queue,!kfifo_is_empty(&mydemo_fifo));
            if(ret){
                    return ret;
            }
    }

    ret = kfifo_to_user(&mydemo_fifo,buf,lbuf,&actual_read);
    if(ret){
            return -EIO;
    }
    if(!kfifo_is_full(&mydemo_fifo))
            wake_up_interruptible(&device.write_queue);

	dprintk("%s: actual_read = %d, pos = %lld\n",__func__,actual_read,*ppos);
	return actual_read;
}


static ssize_t demodrv_write(struct file *file, const char __user *buf, size_t lbuf, loff_t *ppos){

	dprintk("%s enter\n",__func__);
	unsigned int actual_write;
	int ret;
    if(kfifo_is_full(&mydemo_fifo)){
            if(file->f_flags & O_NONBLOCK){
                    return -EAGAIN;
            }
            dprintk("%s :pid=%d, going to sleep\n",__func__,current->pid);
            ret = wait_event_interruptible(device.write_queue,!kfifo_is_full(&mydemo_fifo));
            if(ret)
                    return ret;
    }

    ret = kfifo_from_user(&mydemo_fifo,buf,lbuf,&actual_write);
    if(ret){
            return -EIO;
    }
    if(!kfifo_is_empty(&mydemo_fifo)){
            wake_up_interruptible(&device.read_queue);
    }

	printk("%s:actual_write = %d, ppos=%lld\n",__func__,actual_write,*ppos);
	return actual_write;

	return actual_write;
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
    init_waitqueue_head(&device.read_queue);
    init_waitqueue_head(&device.write_queue);
	ret = misc_register(&mydemodrv_misc_device);
	if(ret){
		printk("failed register misc device\n");
		return ret;
	}
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
