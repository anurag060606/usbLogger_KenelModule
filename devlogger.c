#include<linux/module.h>
#include<linux/kernel.h>
#include <linux/usb.h>
#include<linux/ktime.h>
#include<linux/fs.h>
#include<linux/uaccess.h>
#include<linux/device.h>
#include<linux/cdev.h>

#define BUF_SIZE 256
static char event_buf[BUF_SIZE];
static int event_ready =0; //true false type
static int major_number;
static struct class *dev_class;
static struct cdev my_cdev;

#define DEVICE_NAME "devlogger"
#define CLASS_NAME "devlogger_class"

static int dev_open(struct inode *inode, struct file *flip)
{
	return 0;
}

static ssize_t dev_read(struct file *filp, char __user *buf, size_t count, loff_t *offset)
{
	int len;
	if(!event_ready){
	return 0;
	}

	len=strlen(event_buf);
	if(count<len){
		len=count;
	}
	if(copy_to_user(buf, event_buf, len)){
		return -EFAULT;
	}
	event_ready=0;
	return len;
}
static int dev_release (struct inode *inode, struct file *filp){
	return 0;
}

static struct file_operations fops={
	.owner=THIS_MODULE,
	.open=dev_open, 
	.read=dev_read,
	.release=dev_release,
};

static int usb_notify(struct notifier_block *self, unsigned long action, void *data)
{
	struct usb_device *udev=(struct usb_device *)data;
	ktime_t now=ktime_get_real();
	s64 ns=ktime_to_ns(now);
	s64 sec =ns/1000000000LL;
	s64 usec=(ns%1000000000LL)/1000;

	if(action==USB_DEVICE_ADD){
		snprintf(event_buf, BUF_SIZE, "Connect %lld.%06lld %04x %04x\n", sec,usec, le16_to_cpu(udev->descriptor.idVendor), le16_to_cpu(udev->descriptor.idProduct));
	}else if(action==USB_DEVICE_REMOVE){
		snprintf(event_buf, BUF_SIZE, "Disconnect %lld.%06lld %04x %04x\n", sec,usec, le16_to_cpu(udev->descriptor.idVendor), le16_to_cpu(udev->descriptor.idProduct));
	}else {
		return NOTIFY_OK;
	}

	event_ready=1;

	printk(KERN_INFO "Devlogger: %s", event_buf);
	return NOTIFY_OK;
}

static struct notifier_block usb_nb={
    .notifier_call = usb_notify,
};

static int __init devlogger_init(void)
{
    dev_t dev_num;
    if (alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME) < 0) {
        printk(KERN_ERR "DevLogger: Failed to allocate major number\n");
        return -1;
    }
    major_number = MAJOR(dev_num);
    dev_class = class_create(CLASS_NAME);
    if (IS_ERR(dev_class)) {
        unregister_chrdev_region(dev_num, 1);
        return PTR_ERR(dev_class);
    }
    if (IS_ERR(device_create(dev_class, NULL, dev_num, NULL, DEVICE_NAME))) {
        class_destroy(dev_class);
        unregister_chrdev_region(dev_num, 1);
        return -1;
    }
    cdev_init(&my_cdev, &fops);
    my_cdev.owner = THIS_MODULE;

	if (cdev_add(&my_cdev, dev_num, 1) < 0) {
        device_destroy(dev_class, dev_num);
        class_destroy(dev_class);
        unregister_chrdev_region(dev_num, 1);
        return -1;
    }

    usb_register_notify(&usb_nb);

    printk(KERN_INFO "DevLogger: Loaded. /dev/%s created (major=%d)\n",
           DEVICE_NAME, major_number);
    return 0;
}

static void __exit devlogger_exit(void)
{
    dev_t dev_num = MKDEV(major_number, 0);

    usb_unregister_notify(&usb_nb);

    cdev_del(&my_cdev);
    device_destroy(dev_class, dev_num);
    class_destroy(dev_class);
    unregister_chrdev_region(dev_num, 1);

    printk(KERN_INFO "DevLogger: Unloaded.\n");
}

module_init(devlogger_init);
module_exit(devlogger_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("You");
MODULE_DESCRIPTION("USB Event Logger via /dev/devlogger");
