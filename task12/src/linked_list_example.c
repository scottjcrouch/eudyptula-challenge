// SPDX-License-Identifier: GPL-2.0
/*
 * Example module for demonstrating linked lists
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/list.h>
#include <linux/slab.h>

MODULE_LICENSE("Dual BSD/GPL");

struct identity {
	char name[20];
	int id;
	bool busy;
	struct list_head list_member;
};

static LIST_HEAD(identities);

int identity_create(char *name, int id)
{
	struct identity *new = kmalloc(sizeof(struct identity), GFP_KERNEL);

	if (!new)
		return -ENOMEM;

	strncpy(new->name, name, sizeof(((struct identity *)0)->name));
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

void identity_destroy(int id)
{
	struct identity *entry, *next;

	list_for_each_entry_safe (entry, next, &identities, list_member) {
		if (entry->id == id) {
			list_del(&(entry->list_member));
			kfree(entry);
		}
	}
}

static int linked_list_example_init(void)
{
	struct identity *temp;

	pr_debug("HELLO\n");

	identity_create("Alice", 1);
	identity_create("Bob", 2);
	identity_create("Dave", 3);
	identity_create("Gena", 10);

	temp = identity_find(3);
	pr_debug("id 3 = %s\n", temp->name);

	temp = identity_find(42);
	if (temp == NULL)
		pr_debug("id 42 not found\n");

	identity_destroy(2);
	identity_destroy(1);
	identity_destroy(10);
	identity_destroy(42);
	identity_destroy(3);

	return 0;
}

static void linked_list_example_exit(void)
{
	pr_debug("GOODBYE\n");
}

module_init(linked_list_example_init);
module_exit(linked_list_example_exit);
