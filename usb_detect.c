#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/usb.h>
#include<linux/notifier.h>
#include<linux/ktime.h>
#include<linux/types.h>

static int usb_notify (struct notifier_block *self, unsigned long action, void* data)
{
	struct usb_device *udev= (struct usb_device *) data;
	ktime_t now = ktime_get_real();
	u64 ts= ktime_to_ns(now);

	if (action==USB_DEVICE_ADD){
		printk(KERN_INFO"Devlogger: CONNECTED %llu %04x %04x\n", ts, le16_to_cpu(udev->descriptor.idVendor), le16_to_cpu(udev->descriptor.idProduct));
	}
	else if(action==USB_DEVICE_REMOVE){
		printk(KERN_INFO "Devlogger: DISCONNECTED %llu %04x %04x\n", ts, le16_to_cpu(udev->descriptor.idVendor), le16_to_cpu(udev->descriptor.idProduct));
	}
	return NOTIFY_OK;
}
static struct notifier_block usb_nb={
	.notifier_call=usb_notify,
};

static int __init usb_detect_init(void){
	usb_register_notify(&usb_nb);
	printk(KERN_INFO "Devlogger: USB notifier registered \n");
	return 0;
}

static void __exit usb_detect_exit(void)
{
	usb_unregister_notify(&usb_nb);
	printk(KERN_INFO "Devlogger: USB notifier unregistered\n");
}

module_init(usb_detect_init);
module_exit(usb_detect_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("YOU");
MODULE_DESCRIPTION("USB Event detector");
