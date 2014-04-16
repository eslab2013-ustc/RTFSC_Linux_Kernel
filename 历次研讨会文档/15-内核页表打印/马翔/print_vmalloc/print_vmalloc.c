#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/list.h>
#include <linux/vmalloc.h>

#define VM_LAZY_FREE 		0x01
#define VM_LAZY_FREEING 	0x02
#define VMAP_AREA_LIST_ADDR 	0xc1908068

static struct list_head *vmap_area_list_p = (struct list_head *)VMAP_AREA_LIST_ADDR;

static void * my_seq_start(struct seq_file *m, loff_t *pos)
{
	loff_t n = *pos;
	struct vmap_area *va = list_entry(vmap_area_list_p->next, struct vmap_area, list);
	
	while(n > 0 && &va->list != vmap_area_list_p)
	{
		n --;
		va = list_entry(va->list.next, struct vmap_area, list);
	}

	if(!n && &va->list != vmap_area_list_p)
		return va;
	return NULL;
}

static void * my_seq_next(struct seq_file *m, void *v, loff_t *pos)
{
	struct vmap_area *va = (struct vmap_area *)v;
	
	if(va->list.next == vmap_area_list_p)
		return NULL;
	
	(*pos) ++;
	return list_entry(va->list.next, struct vmap_area, list);
}

static void my_seq_stop(struct seq_file *m, void *v)
{
	return;
}

static int my_seq_show(struct seq_file *m, void *v)
{
	struct vmap_area *va = (struct vmap_area *)v;
	if(va->flags & (VM_LAZY_FREE | VM_LAZY_FREEING))
		return 0;
	
	seq_printf(m, "Vmalloc between: [0x%p - 0x%p]\tSize: %ld\n", (void *)va->va_start, (void *)va->va_end, va->va_end - va->va_start);
	
	return 0;
}

static struct seq_operations my_seq_ops = {
	.start 	= my_seq_start,
	.next  	= my_seq_next,
	.stop	= my_seq_stop,
	.show	= my_seq_show
};

static int my_seq_open(struct inode *inode, struct file *file)
{
	return seq_open(file, &my_seq_ops);
}

static struct file_operations my_file_ops = {
	.owner	 = THIS_MODULE,
	.open 	 = my_seq_open,
	.read 	 = seq_read,
	.llseek	 = seq_lseek,
	.release = seq_release
};

static int __init my_init(void)
{
	proc_create("print_vmalloc", 0644, NULL, &my_file_ops);
	printk("==>>>My_Init Success<<<===\n");
	return 0;
}

static void __exit my_exit(void)
{
	remove_proc_entry("print_vmalloc", NULL);
	printk("===>>>My_Exit<<<===\n");
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("SuperMXC");
MODULE_DESCRIPTION("print_vmalloc");
