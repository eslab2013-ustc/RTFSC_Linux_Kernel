#include <linux/init.h>
#include <linux/module.h>

#define __NR_mycall 223
#define SYS_CALL_TABLE_ADDRESS 0xc14b71d0

unsigned long value;
unsigned long *sys_call_table;

static void redo_CR0_16()
{
	asm volatile (
		"movl	%%cr0 , %%eax \n" 
		"andl	$0xfffeffff , %%eax \n" 
		"movl	%%eax , %%cr0"
		:
		:
	);
}

static void undo_CR0_16()
{
	asm volatile (
		"movl 	%%cr0 , %%eax \n"
		"orl 	$0x00010000 , %%eax \n"
		"movl 	%%eax , %%cr0"
		:
		:
	);

}

asmlinkage long sys_mycall()
{
	printk("======>>>>>> Enter SYS_MyCall <<<<<<======\n");	
	return 13011099;
}

static int __init my_module2_init(void)
{
	printk("===>>> MyModule2 Init <<<===\n");
	
	sys_call_table = (unsigned long *)SYS_CALL_TABLE_ADDRESS;

	redo_CR0_16();
	value = sys_call_table[__NR_mycall];
	sys_call_table[__NR_mycall] = (long)sys_mycall;
	undo_CR0_16();

	return 0;
}

static void __exit my_module2_exit(void)
{
	redo_CR0_16();
	sys_call_table[__NR_mycall] = value;
	undo_CR0_16();

	printk("===>>> MyModule2 Exit <<<===\n");
}

module_init(my_module2_init);
module_exit(my_module2_exit);

MODULE_AUTHOR("SuperMXC");
MODULE_DESCRIPTION("MyModule3");
MODULE_LICENSE("GPL");
