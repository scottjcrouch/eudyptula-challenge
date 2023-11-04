#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>

MODULE_LICENSE("Dual BSD/GPL");

static const char id[] = "voidstarfoobar";

/*
 * Return an ID
 */
static ssize_t eudyptula_read(struct file *file, char __user *user, size_t len,
			      loff_t *offset)
{
	ssize_t retval;
	int data_len = ARRAY_SIZE(id) - 1;
	int read_len;

	if (*offset >= data_len) {
		retval = 0; /* EOF */
		goto out;
	}
	read_len = data_len - *offset;
	if (read_len > len)
		read_len = len;
	if (copy_to_user(user, id + *offset, read_len)) {
		retval = -EFAULT;
		goto out;
	}
	*offset += read_len;
	retval = read_len;

out:
	return retval;
}

/*
 * String compare given input to the ID
 */
static ssize_t eudyptula_write(struct file *file, const char __user *user,
			       size_t len, loff_t *offset)
{
	ssize_t retval;
	char write_buf[ARRAY_SIZE(id) - 1];
	int write_len;

	if (*offset >= ARRAY_SIZE(write_buf)) {
		retval = -EFBIG; /* file too big */
		goto out;
	}
	write_len = ARRAY_SIZE(write_buf) - *offset;
	if (write_len > len)
		write_len = len;
	if (copy_from_user(write_buf + *offset, user, write_len)) {
		retval = -EFAULT;
		goto out;
	}
	if (strncmp(write_buf + *offset, id + *offset, write_len)) {
		retval = -EINVAL;
		goto out;
	}
	*offset += write_len;
	retval = write_len;

out:
	return retval;
}

static struct file_operations eudyptula_fops = {
	.read = eudyptula_read,
	.write = eudyptula_write,
};

static struct miscdevice eudyptuladev = {
	.minor = MISC_DYNAMIC_MINOR,
	.fops = &eudyptula_fops,
	.name = "eudyptuladev",
	.nodename = "eudyptula",
};

static int hello_init(void)
{
	if (!misc_register(&eudyptuladev))
		printk(KERN_ALERT "eudyptula device registered\n");
	return 0;
}

static void hello_exit(void)
{
	misc_deregister(&eudyptuladev);
	printk(KERN_ALERT "eudyptula device unregistered\n");
}

module_init(hello_init);
module_exit(hello_exit);
