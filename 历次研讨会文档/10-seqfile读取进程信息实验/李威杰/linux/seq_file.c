
#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <linux/vmalloc.h>
#include <asm/uaccess.h>
#include <linux/seq_file.h>
#include <linux/list.h>
#include <linux/sched.h>


static struct proc_dir_entry *proc_test_entry;
int flag=0;
struct task_struct *tmp_v=&init_task;

static void *proc_seq_start(struct seq_file *s, loff_t * pos)
{
	if(flag!=0)
	{
		flag=0;
		tmp_v=&init_task;
		return NULL;
	}else{

		return tmp_v;
	}
}

static void *proc_seq_next(struct seq_file *s, void *v, loff_t * pos)
{
	tmp_v = (struct task_struct *)v;
	tmp_v=next_task(tmp_v);
	printk("next\n");
	
	if(tmp_v->pid!=init_task.pid)
	{
		(*pos)++;
		return tmp_v;
	}
	else{

		flag=1;
		return NULL;
	}
	
}

static void proc_seq_stop(struct seq_file *s, void *v)
{
	printk("stop\n");
}

static int proc_seq_show(struct seq_file *s, void *v)
{
	printk("show\n");
	//return 0;
	tmp_v = ((struct task_struct *)v);
	
	int i=0;
	for(i=0;i<30;i++)
	{
		seq_printf(s, "%d,%30s", tmp_v->pid,tmp_v->comm);
	
		seq_putc(s, '\n');
	}
	return 0;
}

static struct seq_operations proc_seq_ops = {
	.start = proc_seq_start,
	.next = proc_seq_next,
	.stop = proc_seq_stop,
	.show = proc_seq_show
};

static int proc_seq_open(struct inode *inode, struct file *file)
{
	return seq_open(file, &proc_seq_ops);
};

static struct file_operations proc_ops = {
	.owner = THIS_MODULE,
	.open = proc_seq_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = seq_release,
};



static int __init proc_seq_init(void)
{
	int ret=0;
	proc_test_entry = create_proc_entry("proc_seq", 0644, NULL);
	if (proc_test_entry == NULL) {
		ret = -ENOMEM;
		pr_err("proc_test: Couldn't create proc entry\n");
	} else {
		proc_test_entry->proc_fops = &proc_ops;
		pr_info("proc_test: Module loaded.\n");
	}
	return ret;
}

static void __exit proc_seq_exit(void)
{
	remove_proc_entry("proc_seq", NULL);
	pr_info("proc_seq, exit ！\n");
}

module_init(proc_seq_init);
module_exit(proc_seq_exit);

MODULE_DESCRIPTION("Procfs seq test module");
MODULE_VERSION("v1.0");
MODULE_AUTHOR("liweijie");
MODULE_LICENSE("Dual BSD/GPL");
