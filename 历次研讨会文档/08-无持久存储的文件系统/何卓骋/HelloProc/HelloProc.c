#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

static int HelloProc_proc_show(struct seq_file *m, void *v)
{
    seq_printf(m, "%s\n", "Hello world!");
    return 0;
}

static int HelloProc_proc_open(struct inode *inode, struct file *file)
{
    return single_open(file, HelloProc_proc_show, NULL);
}

static const struct file_operations HelloProc_proc_fops = {
    .open       = HelloProc_proc_open,
    .read       = seq_read,
    .llseek     = seq_lseek,
    .release    = single_release,
};

static int __init proc_HelloProc_init(void)
{
    proc_create("HelloProc", 0, NULL, &HelloProc_proc_fops);
    return 0;
}

static void __exit proc_HelloProc_exit(void)
{
    remove_proc_entry("HelloProc", NULL);
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("hzc");

module_init(proc_HelloProc_init);
module_exit(proc_HelloProc_exit);

// vim:ts=4:sw=4:tw=0:ft=c:fdm=marker:fdl=10
