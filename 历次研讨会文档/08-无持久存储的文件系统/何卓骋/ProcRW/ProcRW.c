#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include<linux/sched.h>
#include <asm/uaccess.h>
#include <linux/slab.h>

int len, temp;

char *msg;

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
    proc_create("rw", 0, NULL, &rw_proc_fops);
    msg = kmalloc(GFP_KERNEL, 20*sizeof(char));
    return 0;
}

static void __exit proc_rw_exit(void)
{
    remove_proc_entry("rw", NULL);
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("hzc");

module_init(proc_rw_init);
module_exit(proc_rw_exit);

// vim:ts=4:sw=4:tw=0:ft=c:fdm=marker:fdl=10
