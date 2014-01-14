#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/pid.h>
#include <linux/sched.h>


static int my_seq_open(struct inode *, struct file *);
static void * my_seq_start(struct seq_file *, loff_t *);
static void my_seq_stop(struct seq_file *, void *);
static void *my_seq_next(struct seq_file *, void *, loff_t *);
static int my_seq_show(struct seq_file *, void *);

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
	struct task_struct *p ;

	p = &(init_task) ;

/*	for_each_process(p)
	{
		if(cur == *pos)
			return p ;
		cur++ ;
	}
*/

	list_for_each_entry(p, &(p->children), sibling)
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
	struct task_struct *p ;
	p = (struct task_struct*)v;
//	p = list_entry(p->sibling.next, typeof(*p), sibling) ;
//        q = next_task(p) ;
	if(p == &init_task)
		return NULL;
	
	(*pos) ++;
	return next_task(p);
}

static int my_seq_show(struct seq_file *m, void *v)
{
	struct task_struct *p ;
	p = (struct task_struct *)v;
	seq_printf(m, "%6d\t%6d\t%20s\n", p->pid, p->parent->pid, p->comm) ;
//	list_for_each_entry(q, &(p->children), sibling)
//		seq_printf("-->%6d\t%6d\t%20s", q->pid, q->parent->pid, q->comm);
	return 0;
}

// Init Function
static int __init my_init(void)
{
	entry = create_proc_entry("my_seq_file", 0644, NULL);
	
	if(entry)	
		entry->proc_fops = &my_file_ops;
	
	printk("===>>>My_Init Success<<<===\n");
	return 0;
}

// Exit Function
static void __exit my_exit(void)
{
        remove_proc_entry("my_seq_file", NULL) ;
	printk("===>>>My_Exit<<<===\n");
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("hangc");
MODULE_DESCRIPTION("My_Seq_File");
