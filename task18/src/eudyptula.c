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
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/mutex.h>

#if 0
#define MY_DEBUG(str, ...)                                              \
    pr_alert("%s:%s():%d: " str "\n",                                   \
             THIS_MODULE->name,                                         \
             __FUNCTION__,                                              \
             __LINE__,                                                  \
             ##__VA_ARGS__)
#else
#define MY_DEBUG(str, ...)
#endif

#define ID_NAME_BUF_LEN 20

struct identity {
	char name[ID_NAME_BUF_LEN];
	int id;
	bool busy;
	struct list_head list_member;
};

static DEFINE_MUTEX(identities_lock);
static LIST_HEAD(identities);
static int id_counter = 0;

static int identity_create(char *name, int id)
{
	struct identity *new = kmalloc(sizeof(*new), GFP_KERNEL);

	if (!new)
		return -ENOMEM;

	strscpy(new->name, name, sizeof(new->name));
	new->id = id;
	new->busy = false;

	list_add(&(new->list_member), &identities);

	return 0;
}

struct identity *identity_find(int id)
{
	struct identity *entry;

	list_for_each_entry (entry, &identities, list_member) {
		if (entry->id == id)
			return entry;
	}

	return NULL;
}

static void identity_destroy(int id)
{
	struct identity *entry, *next;

	list_for_each_entry_safe (entry, next, &identities, list_member) {
		if (entry->id == id) {
			list_del(&(entry->list_member));
			kfree(entry);
		}
	}
}

static struct identity *identity_get(void)
{
	MY_DEBUG("Checking for new identities");
	if (list_empty(&identities)) {
		MY_DEBUG("None found");
		return NULL;
        }
	return list_first_entry(&identities, struct identity, list_member);
}

DECLARE_WAIT_QUEUE_HEAD(wee_wait);
static struct task_struct *eudyptula_kthread;

static int do_stuff(void *data)
{
	struct identity *next;

	MY_DEBUG("Kthread now running");

	while (!kthread_should_stop()) {
		MY_DEBUG("Entering wait...");
                wait_event_interruptible(wee_wait, kthread_should_stop() || identity_get());
		MY_DEBUG("Woke up!");
		mutex_lock_interruptible(&identities_lock);
		if ((next = identity_get())) {
			pr_alert("Identity.name: %s\n", next->name);
			pr_alert("Identity.id: %d\n", next->id);
			identity_destroy(next->id);
		}
		mutex_unlock(&identities_lock);
		if (next)
			msleep(500);
	}

	MY_DEBUG("Kthread stopping");

	return 0;
}

static ssize_t eudyptula_write(struct file *file, const char __user *user,
			       size_t len, loff_t *offset)
{
	ssize_t retval;
	char write_buf[ID_NAME_BUF_LEN] = { 0 };
	int truncated_len = min(len, (size_t)ID_NAME_BUF_LEN);

	MY_DEBUG("Writing...");

	if (copy_from_user(write_buf, user, truncated_len)) {
		MY_DEBUG("Failed copy during writing");
		retval = -EFAULT;
		goto out;
	}
	if (mutex_lock_interruptible(&identities_lock)) {
		MY_DEBUG("Woken when trying to get lock during write");
		retval = -ERESTARTSYS;
		goto out;
	}
	if (identity_create(write_buf, id_counter++)) {
		MY_DEBUG("Failed creating new identity");
		retval = -ENOMEM;
		goto unlock;
	}
	*offset += truncated_len;
	retval = len; /* truncate silently rather than indicate a partial write */

	MY_DEBUG("Finished writing, now waking up kthread");

	wake_up(&wee_wait);

unlock:
	mutex_unlock(&identities_lock);
out:
	return retval;
}

static struct file_operations eudyptula_fops = {
	.owner = THIS_MODULE,
	.write = eudyptula_write,
};

static struct miscdevice eudyptuladev = {
	.minor = MISC_DYNAMIC_MINOR,
	.fops = &eudyptula_fops,
	.name = "eudyptuladev",
	.nodename = "eudyptula",
	.mode = S_IWUGO,
};

static int __init eudyptula_init(void)
{
	int ret;

	MY_DEBUG("Module loading...");

	eudyptula_kthread = kthread_run(do_stuff, NULL, "eudyptula");
	if (IS_ERR(eudyptula_kthread)) {
		MY_DEBUG("Failed to create \"eudyptula\" kthread during module load");
		return PTR_ERR(eudyptula_kthread); /* -ENOMEM */
	}

	ret = misc_register(&eudyptuladev);
	if (ret) {
		MY_DEBUG("Failed to register \"eudyptula\" device during module load");
		kthread_stop(eudyptula_kthread);
		return ret;
	}

	MY_DEBUG("Module loaded");

	return 0;
}

static void __exit eudyptula_exit(void)
{
	int ret;
	struct identity *iter;

	misc_deregister(&eudyptuladev);

	ret = kthread_stop(eudyptula_kthread);
	MY_DEBUG("kthread_stop() returned %d", ret);

	while ((iter = identity_get()))
		identity_destroy(iter->id);
}

module_init(eudyptula_init);
module_exit(eudyptula_exit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Scott J. Crouch");
MODULE_DESCRIPTION("Example use of a kthread to defer work using a wait queue");
