#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include<linux/sched.h>
#include <asm/uaccess.h>
#include <linux/slab.h>

int len, temp;
char *msg;
static struct proc_dir_entry *proc_parent;

int proc_rw_read(struct file *filp, char *buf, size_t count, loff_t *offp)
{
    if (count>temp) {
        count = temp;
    }
    temp = temp-count;
    copy_to_user(buf, msg, count);
    if (count == 0)
        temp = len;
    return count;
}

int proc_rw_write(struct file *filp, char *buf, size_t count, loff_t *offp)
{
    copy_from_user(msg, buf, count);
    len = count;
    temp = len;
    return count;
}

static const struct file_operations rw_proc_fops = {
    read: proc_rw_read,
    write: proc_rw_write
};

static int __init proc_rw_init(void)
{
    proc_parent = proc_mkdir_mode("proctest", 0777, NULL);
    if(!proc_parent)
    {
        printk(KERN_INFO "Error creating proc entry");
        return -ENOMEM;
    }
    proc_create("rw", 0666, proc_parent, &rw_proc_fops);
    msg = kmalloc(GFP_KERNEL, 2*sizeof(char));
    return 0;
}

static void __exit proc_rw_exit(void)
{
    remove_proc_entry("rw", proc_parent);
    remove_proc_entry("proctest", NULL);
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("hzc");

module_init(proc_rw_init);
module_exit(proc_rw_exit);

// vim:ts=4:sw=4:tw=0:ft=c:fdm=marker:fdl=10
