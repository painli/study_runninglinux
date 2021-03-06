#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/kfifo.h>
#include <linux/poll.h>


struct mydemo_device{
        char name[16];
        struct device *dev;
};


struct mydemo_private_data{
        struct mydemo_device *device;
        char name[64];
        wait_queue_head_t read_queue;
        wait_queue_head_t write_queue;
        struct kfifo mydemo_fifo;
};
#define MYDEMO_DEVICES_COUNT 8
#define MYDEMO_NAME "my_demo_drv"
#define MYDEMO_FIFO_SIZE 64
static struct mydemo_device *mydemo_device[MYDEMO_DEVICES_COUNT];

static struct cdev *demo_cdev;
static dev_t devt;
static int demodrv_open(struct inode *inode, struct file *file){
        unsigned int minor = iminor(inode);
        struct mydemo_private_data *data;
        struct mydemo_device *device = mydemo_device[minor];
        int ret;
        printk(KERN_ERR "%s: major=%d, monor=%d, device=%s \n",__func__,MAJOR(inode->i_rdev),MINOR(inode->i_rdev),device->name);

        data = kmalloc(sizeof(struct mydemo_private_data),GFP_KERNEL);
        if(!data){
                return -ENOMEM;
        }
        sprintf(data->name,"private_data_%d",minor);
        ret = kfifo_alloc(&data->mydemo_fifo,MYDEMO_FIFO_SIZE,GFP_KERNEL);
        if(ret){
                kfree(data);
                return -ENOMEM;
        }
        init_waitqueue_head(&data->read_queue);
        init_waitqueue_head(&data->write_queue);

        data->device = device;
        file->private_data = data;
        return 0;
}



static unsigned int demodrv_poll(struct file *file, struct poll_table_struct * wait){
        int mask = 0;
        printk(KERN_ERR "%s:%d pid=%d enter\n",__func__,__LINE__,current->pid);
        struct mydemo_private_data* data = file->private_data;
        struct mydemo_device* device = data->device;

        poll_wait(file,&data->read_queue,wait);
        poll_wait(file,&data->write_queue,wait);

        if(!kfifo_is_empty(&data->mydemo_fifo)){
                mask |= POLLIN | POLLRDNORM;
                printk(KERN_ERR "%s:%d POLLIN\n",__func__,__LINE__);
        }
        if(!kfifo_is_full(&data->mydemo_fifo)){
                mask |= POLLOUT | POLLWRNORM;
                printk(KERN_ERR "%s:%d POLLOUT\n",__func__,__LINE__);
        }
        return mask;
}
static int demodrv_release(struct inode *inode, struct file *file){

        return 0;
}

static ssize_t demodrv_read(struct file *file, char __user *buf, size_t lbuf, loff_t *ppos){

        printk(KERN_ERR "%s enter\n",__func__);

        int actual_read;
        int ret;
        struct mydemo_private_data* data = file->private_data;
        struct mydemo_device* device = data->device;


        if(kfifo_is_empty(&data->mydemo_fifo)){
                if(file->f_flags & O_NONBLOCK){
                        return -EAGAIN;
                }
                printk(KERN_ERR "%s: pid=%d,going to sleep\n",__func__,current->pid);
                ret = wait_event_interruptible(data->read_queue,!kfifo_is_empty(&data->mydemo_fifo));
                if(ret){
                        return ret;
                }
        }


        ret = kfifo_to_user(&data->mydemo_fifo,buf,lbuf,&actual_read);
        if(ret){
                return -EIO;
        }
        if(!kfifo_is_full(&data->mydemo_fifo)){
                printk(KERN_ERR "%s:%d pid = %d ,write_queue read_queue\n",__func__,__LINE__);
                wake_up_interruptible(&data->write_queue);
        }


        printk(KERN_ERR "%s: actual_read = %d, pos = %lld\n",__func__,actual_read,*ppos);
        return actual_read;
}

static ssize_t demodrv_write(struct file *file, const char __user *buf, size_t lbuf, loff_t *ppos){

        printk(KERN_ERR "%s enter \n",__func__);

        unsigned int actual_write;
        int ret;
        struct mydemo_private_data* data = file->private_data;
        struct mydemo_device* device = data->device;
        if(kfifo_is_full(&data->mydemo_fifo)){
                if(file->f_flags & O_NONBLOCK ){
                        return -EAGAIN;
                }
                printk(KERN_ERR "%s : pid=%d, going to sleep\n",__func__,current->pid);
                ret = wait_event_interruptible(data->write_queue,!kfifo_is_full(&data->mydemo_fifo));
                if(ret)
                        return ret;
        }
        
        ret = kfifo_from_user(&data->mydemo_fifo,buf,lbuf,&actual_write);
        if(ret){
                printk(KERN_ERR "kfifo from user error\n");
                return -EIO;
                
        }

        if(!kfifo_is_empty(&data->mydemo_fifo)){
                printk(KERN_ERR "%s:%d pid = %d ,wake_up read_queue\n",__func__,__LINE__);
                wake_up_interruptible(&data->read_queue);
        }

        printk(KERN_ERR "%s: actual_write=%d,ppos=%lld\n",__func__,actual_write,*ppos);
        return actual_write;
}

static const struct file_operations demodrv_fops = {
        .owner = THIS_MODULE,
        .open = demodrv_open,
        .release = demodrv_release,
        .read = demodrv_read,
        .write = demodrv_write,
        .poll = demodrv_poll,
};

static int __init simple_char_init(void){
        int ret;
        int i;
        struct mydemo_device *device;

        ret = alloc_chrdev_region(&devt,0,MYDEMO_DEVICES_COUNT,MYDEMO_NAME);
        if(ret){
                printk(KERN_ERR,"failed to allocate char device region");
                return ret;
        }
        demo_cdev = cdev_alloc();
        if(!demo_cdev){
                printk(KERN_ERR,"cdev_alloc failed");
                goto unregister_chrdev;
        }
        cdev_init(demo_cdev,&demodrv_fops);

        ret = cdev_add(demo_cdev,devt,MYDEMO_DEVICES_COUNT);

        if(ret){
                printk(KERN_ERR "cdev add failed");
                goto cdev_failed;
        }
        for(i = 0; i < MYDEMO_DEVICES_COUNT; i++){
                device = kmalloc(sizeof(struct mydemo_device),GFP_KERNEL);
                if(!device){
                        ret = -ENOMEM;
                        goto free_device;
                }
                sprintf(device->name,"%s%d",MYDEMO_NAME,i);
                mydemo_device[i] = device;
        }
        printk(KERN_ERR "succeeded register char device:%s \n",MYDEMO_NAME);
        return 0;
free_device:
        for(i=0; i < MYDEMO_DEVICES_COUNT;i++){
               if(mydemo_device[i])
                    kfree(mydemo_device[i]);
        }
cdev_failed:
        cdev_del(demo_cdev);
unregister_chrdev:
        unregister_chrdev(devt,MYDEMO_DEVICES_COUNT);
        
        return ret;
}

static void __init simple_char_exit(void){
        int i;

        for(i=0; i < MYDEMO_DEVICES_COUNT;i++){
               if(mydemo_device[i])
                    kfree(mydemo_device[i]);
        }

        cdev_del(demo_cdev);

        unregister_chrdev(devt,MYDEMO_DEVICES_COUNT);
}

module_init(simple_char_init);
module_exit(simple_char_exit);



MODULE_AUTHOR("pain.li");
MODULE_LICENSE("GPL V2");
MODULE_DESCRIPTION("simple character device");
