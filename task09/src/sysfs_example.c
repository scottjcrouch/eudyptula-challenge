// SPDX-License-Identifier: GPL-2.0
/*
 * Example use of sysfs.
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/jiffies.h>
#include <linux/gfp.h>
#include <linux/sysfs.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/stat.h>
#include <asm/page.h>

static ssize_t eudyptula_show(struct kobject *kobj, struct attribute *attr,
			      char *buf);
static ssize_t eudyptula_store(struct kobject *kobj, struct attribute *attr,
			       const char *buf, size_t size);
static void eudyptula_dev_release(struct kobject *kobj);

struct eudyptula_dev {
	struct mutex foo_mut;
	void *foo_buf;
	ssize_t foo_size;
	struct kobject kobj;
};

static struct eudyptula_dev *eudyptula_dev;

static struct attribute id_attr = {
	.name = "id",
	.mode = S_IRUGO | S_IWUGO,
};
static struct attribute jiffies_attr = {
	.name = "jiffies",
	.mode = S_IRUGO,
};
static struct attribute foo_attr = {
	.name = "foo",
	.mode = S_IRUGO | S_IWUSR,
};

static const struct sysfs_ops eudyptula_ops = {
	.show = eudyptula_show,
	.store = eudyptula_store,
};

static struct attribute *eudyptula_attrs[] = {
	&id_attr,
	&jiffies_attr,
	&foo_attr,
	NULL,
};

static struct kobj_type eudyptula_ktype = {
	.release = eudyptula_dev_release,
	.sysfs_ops = &eudyptula_ops,
	.default_attrs = eudyptula_attrs,
};

static const char id[] = "voidstarfoobar";
static ssize_t id_show(char *buf)
{
	int data_len = ARRAY_SIZE(id) - 1;
	memcpy(buf, id, data_len);
	return data_len;
}

static ssize_t id_store(const char *buf, size_t size)
{
	if (size == strlen(id) || strncmp(buf, id, size))
		return -EINVAL;
	return size;
}

#define MAX_JIF_STR (2 + (2 * sizeof(jiffies)) + 1 + 1)
static ssize_t jiffies_show(char *buf)
{
	unsigned long current_jif = jiffies;
	return snprintf(buf, MAX_JIF_STR, "%#lx\n", current_jif);
}

static ssize_t foo_show(char *buf)
{
	if (mutex_lock_interruptible(&eudyptula_dev->foo_mut))
		return -ERESTARTSYS;
	memcpy(buf, eudyptula_dev->foo_buf, eudyptula_dev->foo_size);
	mutex_unlock(&eudyptula_dev->foo_mut);
	return eudyptula_dev->foo_size;
}

static ssize_t foo_store(const char *buf, size_t size)
{
	if (mutex_lock_interruptible(&eudyptula_dev->foo_mut))
		return -ERESTARTSYS;
	memcpy(eudyptula_dev->foo_buf, buf, size);
	eudyptula_dev->foo_size = size;
	mutex_unlock(&eudyptula_dev->foo_mut);
	return size;
}

static ssize_t eudyptula_show(struct kobject *kobj, struct attribute *attr,
			      char *buf)
{
	if (attr->name == id_attr.name)
		return id_show(buf);
	else if (attr->name == jiffies_attr.name)
		return jiffies_show(buf);
	else if (attr->name == foo_attr.name)
		return foo_show(buf);
	else
		return -EINVAL;
}

static ssize_t eudyptula_store(struct kobject *kobj, struct attribute *attr,
			       const char *buf, size_t size)
{
	if (attr->name == id_attr.name)
		return id_store(buf, size);
	else if (attr->name == jiffies_attr.name)
		return -EPERM;
	else if (attr->name == foo_attr.name)
		return foo_store(buf, size);
	else
		return -EINVAL;
}

static void eudyptula_dev_release(struct kobject *kobj)
{
	struct eudyptula_dev *dev =
		container_of(kobj, struct eudyptula_dev, kobj);
	free_page((unsigned long)dev->foo_buf);
	kfree(dev);
}

static int eudyptula_init(void)
{
	int retval;

	eudyptula_dev = kmalloc(sizeof(struct eudyptula_dev), GFP_KERNEL);
	eudyptula_dev->foo_buf = (void *)get_zeroed_page(GFP_KERNEL);
	if (!eudyptula_dev->foo_buf) {
		return -ENOMEM;
	}
	eudyptula_dev->foo_size = 0;
	mutex_init(&eudyptula_dev->foo_mut);
	kobject_init(&eudyptula_dev->kobj, &eudyptula_ktype);

	retval = kobject_add(&eudyptula_dev->kobj, NULL, "eudyptula");
	if (retval) {
		kobject_put(&eudyptula_dev->kobj);
		return retval;
	}

	return 0;
}

static void eudyptula_exit(void)
{
	kobject_put(&eudyptula_dev->kobj);
}

module_init(eudyptula_init);
module_exit(eudyptula_exit);

MODULE_AUTHOR("Scott J. Crouch");
MODULE_DESCRIPTION("Example use of sysfs");
MODULE_LICENSE("Dual BSD/GPL");
