#include <linux/init.h>
#include <linux/module.h>

static int __init my_module1_init(void)
{
	printk("===>>> MyModule1 Init <<<===\n");
	return 0;
}

static void __exit my_module1_exit(void)
{
	printk("===>>> MyModule1 Exit <<<===\n");
}

module_init(my_module1_init);
module_exit(my_module1_exit);

MODULE_AUTHOR("SuperMXC");
MODULE_DESCRIPTION("MyModule1");
MODULE_LICENSE("GPL");
