#include <linux/module.h>
#include <linux/kernel.h>

static int __init tst_init(void)
{
	printk(KERN_INFO"Hello world!\n");
	return 0;
}

static void __exit tst_exit(void)
{
	printk(KERN_INFO"Goodbye world!\n");
}

module_init(tst_init);
module_exit(tst_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Cédric Chépied");
MODULE_DESCRIPTION("KHello World!");
