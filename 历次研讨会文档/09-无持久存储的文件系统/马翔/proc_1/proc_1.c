#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/proc_fs.h>

#define LIST_SIZE 1000

struct LNode
{
	struct list_head list;
	char buf[10];
};

static struct list_head my_list = {
	.next = &my_list,
	.prev = &my_list,
};
static struct proc_dir_entry *entry;

int my_read_proc(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int len = 0;
	struct LNode *p;

	list_for_each_entry(p, &my_list, list)
	{
		len += sprintf(page + len, p->buf);
	}

	*eof = 1;	
	
	return len;
}

static int __init my_init(void)
{
	struct LNode *p;
	int i;

	entry = create_proc_entry("proc_1", 0644, NULL);
	
	if(entry)	
		entry->read_proc = my_read_proc;
	
	for(i = 0;i < LIST_SIZE;i ++)
	{
		p = (struct LNode *)kmalloc(sizeof(struct LNode), GFP_KERNEL);
		
		sprintf(p->buf, "Node: %d\n", i);
		list_add_tail(&p->list, &my_list);
	}
	
	printk("===>>>My_Init Success<<<===\n");
	return 0;
}

static void __exit my_exit(void)
{
	struct list_head *p = my_list.next;
	struct LNode *tmp;
	int i;
	for(i = 0;i < LIST_SIZE;i ++)
	{
		tmp = list_entry(p, struct LNode, list);
		p = p->next;
		kfree(tmp);
	}
	remove_proc_entry("proc_1", NULL);

	printk("===>>>My_Exit<<<===\n");
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("SuperMXC");
MODULE_DESCRIPTION("Proc_1");
