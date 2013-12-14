#include<linux/init.h>
#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/list.h>
#include<linux/sched.h>
#include<linux/slab.h>
#include<linux/proc_fs.h>
#include<linux/seq_file.h>


static int myOpen(struct inode *inode,struct file *file);
static void *seqFileStart(struct seq_file *file,loff_t *pos);
static void seqFileStop(struct seq_file *m ,void *v);
static void *seqFileNext(struct seq_file *m, void *v,loff_t *pos);
static int seqFileShow(struct seq_file *m,void *v);
static void myChild(struct seq_file *m, struct task_struct *self);

static struct proc_dir_entry *entry;

static struct seq_operations seqFileOps={
	.start		=seqFileStart,
	.stop		=seqFileStop,
	.next		=seqFileNext,
	.show		=seqFileShow,
};


static const struct file_operations myFileOps={
	.owner		=THIS_MODULE,
	.open		=myOpen,
	.read		=seq_read,
	.llseek	=seq_lseek,
	.release	=seq_release,

};
static void myChild(struct seq_file *m, struct task_struct *self)
{
	struct list_head *p=&(self->children);
	struct list_head *sibg,*flag;
	struct task_struct *tmp;
	if(p->next==p)
		seq_printf(m,"I have no children\n");
	else
	{
		tmp=list_entry(p,struct task_struct,children);
		//seq_printf(m,"%12d",tmp->pid);
		sibg=&(tmp->sibling);
		flag=&(tmp->sibling);
		do{
			tmp=list_entry(sibg,struct task_struct,sibling);
			seq_printf(m,"%12d",tmp->pid);
			sibg=sibg->next;
		}while(sibg!=flag);
	}
	seq_printf(m,"\n");
}
static int myOpen(struct inode *inode,struct file *file)
{
	return seq_open(file,&seqFileOps);
}

static void *seqFileStart(struct seq_file *m,loff_t *pos)
{
	loff_t cur=0;
	struct list_head *p=&(init_task.tasks);
	do
	{
		if(cur==*pos)
			return list_entry(p,struct task_struct,tasks);
			cur++;
			p=p->next;
	}while(p!=&(init_task.tasks));
	return NULL;
}

static void seqFileStop(struct seq_file *m ,void *v)
{
	return ;
}

static void *seqFileNext(struct seq_file *m, void *v,loff_t *pos)
{
	struct list_head *p=(((struct task_struct *)v)->tasks).next;
	if(p==&(init_task.tasks))
		return NULL;
	(*pos)++;
	return list_entry(p,struct task_struct,tasks);
}

static int seqFileShow(struct seq_file *m,void *v)
{
	struct task_struct *p=(struct task_struct *)v;
	seq_printf(m,"the process's pid and tgid is %d  %d  Name is %s  ",p->pid,p->tgid,p->comm);
	if(p->parent==NULL)
		seq_printf(m,"NO parent    ");
	else
		seq_printf(m,"My parent is %d    ",p->parent->pid);
	myChild(m, p);
	return 0;
}

static int __init seqInit(void)
{
	entry=create_proc_entry("you",0664,NULL);
	if(entry)
		entry->proc_fops=&myFileOps;
	return 0;
}

static void __exit seqExit(void)
{
	remove_proc_entry("you",NULL);
	printk("====>>>seqFile exit<<<===\n");
}


module_init(seqInit);
module_exit(seqExit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("SuperMXC");
MODULE_DESCRIPTION("you");


