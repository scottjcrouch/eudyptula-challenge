// SPDX-License-Identifier: GPL-2.0
/*
 * hello.c: example module
 */
#include <linux/init.h>
#include <linux/module.h>

MODULE_LICENSE("Dual BSD/GPL");

static int hello_init(void)
{
	pr_alert("Hello world!\n");
	return 0;
}

static void hello_exit(void)
{
	pr_alert("Goodbye world!\n");
}

module_init(hello_init);
module_exit(hello_exit);
