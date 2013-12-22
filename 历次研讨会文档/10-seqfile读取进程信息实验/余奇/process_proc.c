#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

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
	struct list_head *p = &(init_task.tasks);

	do
	{
		if(cur == *pos)
			return list_entry(p, struct task_struct, tasks);
		cur ++;
		p = p->next;

	} while(p != &(init_task.tasks));

	return NULL;
}

static void my_seq_stop(struct seq_file *m, void *v)
{
	return;
}

static void *my_seq_next(struct seq_file *m, void *v, loff_t *pos)
{
	struct list_head *p = (((struct task_struct *)v)->tasks).next;

	if(p == &(init_task.tasks))
		return NULL;

	(*pos) ++;
	return list_entry(p, struct task_struct, tasks);
}

static int my_seq_show(struct seq_file *m, void *v)
{
	struct task_struct *p = (struct task_struct *)v;
	seq_printf(m, "The Pid is %4d , The Task Name is %s\n", p->pid, p->comm);
	return 0;
}

// Init Function
static int __init my_init(void)
{
	entry = create_proc_entry("process_proc", 0644, NULL);

	if(entry)
		entry->proc_fops = &my_file_ops;

	return 0;
}

// Exit Function
static void __exit my_exit(void)
{
	remove_proc_entry("process_proc", NULL);
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("YQ");
MODULE_DESCRIPTION("All_Process");
