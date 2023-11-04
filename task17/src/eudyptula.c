// SPDX-License-Identifier: GPL-2.0

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/wait.h>
#include <linux/kthread.h>
#include <linux/delay.h>

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Scott J. Crouch");
MODULE_DESCRIPTION("Example use of wait queues and kthreads");

static const char id[] = "voidstarfoobar";

/*
 * strncmp input to our ID
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
	.write = eudyptula_write,
};

static struct miscdevice eudyptuladev = {
	.minor = MISC_DYNAMIC_MINOR,
	.fops = &eudyptula_fops,
	.name = "eudyptuladev",
	.nodename = "eudyptula",
	.mode = S_IWUGO,
};

DECLARE_WAIT_QUEUE_HEAD(wee_wait);
static struct task_struct *eudyptula_kthread;

static int do_nothing(void *data)
{
	pr_alert("eudyptula kthread starting\n");
	while (!kthread_should_stop()) {
		wait_event_interruptible(wee_wait, kthread_should_stop());
		pr_alert("eudyptula kthread woke up\n");
	}
	pr_alert("eudyptula kthread stopping\n");

	return 0;
}

static int __init hello_init(void)
{
	int ret;

	eudyptula_kthread = kthread_run(do_nothing, NULL, "eudyptula");
	if (IS_ERR(eudyptula_kthread)) {
		pr_alert("Failed to create \"eudyptula\" kthread\n");
		return PTR_ERR(eudyptula_kthread); /* -ENOMEM */
	}

	ret = misc_register(&eudyptuladev);
	if (ret) {
		pr_alert("Failed to register \"eudyptula\" device\n");
		kthread_stop(eudyptula_kthread);
		return ret;
	}

	return 0;
}

static void __exit hello_exit(void)
{
	int ret = kthread_stop(eudyptula_kthread);
	pr_alert("kthread_stop() returned %d\n", ret);

	misc_deregister(&eudyptuladev);
}

module_init(hello_init);
module_exit(hello_exit);
