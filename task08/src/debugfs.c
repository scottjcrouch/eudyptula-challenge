// SPDX-License-Identifier: GPL-2.0
/*
 * Example module for testing debugfs
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/jiffies.h>
#include <linux/gfp.h>
#include <linux/debugfs.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <asm/page.h>

static const char id[] = "voidstarfoobar";

static ssize_t id_read(struct file *file, char __user *user, size_t len,
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

static ssize_t id_write(struct file *file, const char __user *user, size_t len,
			loff_t *offset)
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

static const struct file_operations id_fops = {
	.read = id_read,
	.write = id_write,
};

#define MAX_JIF_STR (2 + (2 * sizeof(jiffies)) + 1 + 1)
static ssize_t jiffies_read(struct file *file, char __user *user, size_t len,
			    loff_t *offset)
{
	ssize_t retval;
	int read_len;

	if (*offset >= strlen(file->private_data)) {
		retval = 0; /* EOF */
		goto out;
	}
	read_len = strlen(file->private_data) - *offset;
	if (read_len > len)
		read_len = len;
	if (copy_to_user(user, file->private_data, read_len)) {
		retval = -EFAULT;
		goto out;
	}
	*offset += read_len;
	retval = read_len;

out:
	return retval;
}

static int jiffies_open(struct inode *inode, struct file *file)
{
	unsigned long current_jif = jiffies;

	file->private_data = kmalloc(MAX_JIF_STR, GFP_KERNEL);
	if (!file->private_data)
		return -ENOMEM;
	snprintf(file->private_data, MAX_JIF_STR, "%#lx\n", current_jif);
	return 0;
}

static int jiffies_release(struct inode *inode, struct file *file)
{
	kfree(file->private_data);
	return 0;
}

static const struct file_operations jiffies_fops = {
	.read = jiffies_read,
	.open = jiffies_open,
	.release = jiffies_release,
};

static DEFINE_MUTEX(foo_mut);
static void *foo_buf;

static ssize_t foo_read(struct file *file, char __user *user, size_t len,
			loff_t *offset)
{
	ssize_t retval;
	int read_len;

	if (*offset >= PAGE_SIZE)
		return 0; /* EOF */
	read_len = PAGE_SIZE - *offset;
	if (read_len > len)
		read_len = len;
	if (mutex_lock_interruptible(&foo_mut))
		return -ERESTARTSYS;
	if (copy_to_user(user, foo_buf + *offset, read_len)) {
		retval = -EFAULT;
		goto out;
	}
	*offset += read_len;
	retval = read_len;

out:
	mutex_unlock(&foo_mut);
	return retval;
}

static ssize_t foo_write(struct file *file, const char __user *user, size_t len,
			 loff_t *offset)
{
	ssize_t retval;
	int write_len;

	if (*offset >= PAGE_SIZE)
		return -EFBIG; /* file too big */
	write_len = PAGE_SIZE - *offset;
	if (write_len > len)
		write_len = len;
	if (mutex_lock_interruptible(&foo_mut))
		return -ERESTARTSYS;
	if (copy_from_user(foo_buf + *offset, user, write_len)) {
		retval = -EFAULT;
		goto out;
	}
	*offset += write_len;
	retval = write_len;

out:
	mutex_unlock(&foo_mut);
	return retval;
}

static const struct file_operations foo_fops = {
	.read = foo_read,
	.write = foo_write,
};

static struct dentry *eudyptula_dentry;

static int debugfs_init(void)
{
	pr_alert("debugfs module init\n");
	eudyptula_dentry = debugfs_create_dir("eudyptula", NULL);
	if (!eudyptula_dentry)
		return -ENODEV;
	if (!debugfs_create_file("id", 0666, eudyptula_dentry, NULL, &id_fops))
		return -ENODEV;
	if (!debugfs_create_file("jiffies", 0444, eudyptula_dentry, NULL,
				 &jiffies_fops))
		return -ENODEV;
	foo_buf = (void *)get_zeroed_page(GFP_KERNEL);
	if (!foo_buf)
		return -ENOMEM;
	if (!debugfs_create_file("foo", 0644, eudyptula_dentry, NULL,
				 &foo_fops)) {
		free_page((unsigned long)foo_buf);
		return -ENODEV;
	}
	return 0;
}

static void debugfs_exit(void)
{
	debugfs_remove_recursive(eudyptula_dentry);
	free_page((unsigned long)foo_buf);
	pr_alert("debugfs module exit\n");
}

module_init(debugfs_init);
module_exit(debugfs_exit);

MODULE_AUTHOR("Scott J. Crouch");
MODULE_DESCRIPTION("Example use of debugfs");
MODULE_LICENSE("Dual BSD/GPL");
