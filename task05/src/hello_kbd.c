#include <linux/init.h>
#include <linux/module.h>
#include <linux/usb.h>

MODULE_LICENSE("Dual BSD/GPL");

#define CLASS_HID         0x3
#define SUBCLASS_HID_NONE 0x1
#define PROTOCOL_HID_KBD  0x1
static struct usb_device_id kbds[] = {
    { USB_INTERFACE_INFO(CLASS_HID, SUBCLASS_HID_NONE, PROTOCOL_HID_KBD) },
    { },
};
MODULE_DEVICE_TABLE(usb, kbds);

static int hello_init(void)
{
    printk(KERN_ALERT "Hello world!\n");
    return 0;
}

static void hello_exit(void)
{
    printk(KERN_ALERT "Goodbye world!\n");
}

module_init(hello_init);
module_exit(hello_exit);
