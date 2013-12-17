#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

#define LIST_SIZE 1000

static int my_seq_open(struct inode *, struct file *);
static void * my_seq_start(struct seq_file *, loff_t *);
static void my_seq_stop(struct seq_file *, void *);
static void *my_seq_next(struct seq_file *, void *, loff_t *);
static int my_seq_show(struct seq_file *, void *);

struct LNode
{
	struct list_head list;
	char buf[128];
};

static struct list_head my_list = {
	.next = &my_list,
	.prev = &my_list,
};
static struct proc_dir_entry *entry;

// Seq_Operations Structure
static struct seq_operations my_seq_ops = {
	.start	= my_seq_start,
	.stop 	= my_seq_stop,
	.next	= my_seq_next,
	.show	= my_seq_show,
};

// File_Operations Structure
static const struct file_operations my_file_ops = {
	.owner	 = THIS_MODULE,
	.open 	 = my_seq_open,
	.read 	 = seq_read,
	.llseek  = seq_lseek,
	.release = seq_release,
};

// Functions For Var: my_file_ops.open
static int my_seq_open(struct inode *inode, struct file *file)
{
	return seq_open(file, &my_seq_ops);
}

// Functions For Var: my_seq_ops
static void * my_seq_start(struct seq_file *m, loff_t *pos)
{	
	loff_t cur = 0;
	struct LNode *p;

	list_for_each_entry(p, &my_list, list)
	{
		if(cur == *pos)
			return p;
		cur ++;
	}

	return NULL;
}

static void my_seq_stop(struct seq_file *m, void *v)
{
	return;
}

static void *my_seq_next(struct seq_file *m, void *v, loff_t *pos)
{
	struct list_head *p = (((struct LNode *)v)->list).next;
	
	if(p == &my_list)
		return NULL;
	
	(*pos) ++;
	return list_entry(p, struct LNode, list);
}

static int my_seq_show(struct seq_file *m, void *v)
{
	struct LNode *p = (struct LNode *)v;
	seq_printf(m, p->buf);
	return 0;
}

//new Init Function
static int __init my_init(void)
{
	struct LNode *p;
	struct task_struct *tp;
	int i;

	entry = create_proc_entry("ps_read", 0644, NULL);
	
	if(entry)	
		entry->proc_fops = &my_file_ops;
	
	for_each_process(tp)
	{
		p = (struct LNode *)kmalloc(sizeof(struct LNode), GFP_KERNEL);
		sprintf(p->buf,"%d/t/t%d/t/t%s/n",tp->pid,tp->parent->pid,tp->comm);
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
	remove_proc_entry("ps_read", NULL);

	printk("===>>>My_Exit<<<===\n");
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("SuperMXC");
MODULE_DESCRIPTION("Seq_File");
