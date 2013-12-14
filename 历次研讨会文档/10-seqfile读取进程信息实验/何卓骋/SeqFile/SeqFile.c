#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/list.h>
#include <linux/sched.h>

#define PROC_NAME   "SeqProcess"

MODULE_AUTHOR("hzc");
MODULE_LICENSE("GPL");

/**
 * This function is called at the beginning of a sequence.
 * ie, when:
 *  - the /proc file is read (first time)
 *  - after the function stop (end of sequence)
 *
 */
struct task_struct *mcurrent;

static void *my_seq_start(struct seq_file *s, loff_t *pos)
{
    struct task_struct *task = &init_task;

    if (*pos == 0)
        return task;
    if (mcurrent != &init_task) {
        seq_printf(s, "continue\n");
        return next_task(mcurrent);
    }
    *pos = 0;
    return NULL;
}

/**
 * This function is called after the beginning of a sequence.
 * It's called untill the return is NULL (this ends the sequence).
 *
 */
static void *my_seq_next(struct seq_file *s, void *v, loff_t *pos)
{
    struct task_struct *task = next_task((struct task_struct *)v);
    mcurrent = task;
    (*pos)++;
    if (task != &init_task) {
        return task;
    }
    return NULL;
}

/**
 * This function is called at the end of a sequence
 *
 */
static void my_seq_stop(struct seq_file *s, void *v)
{
    /* nothing to do, we use a static value in start() */
}

/**
 * This function is called for each "step" of a sequence
 *
 */
static int my_seq_show(struct seq_file *s, void *v)
{
    struct task_struct *task = (struct task_struct *)v;
    int i;
    for (i = 0; i < 20; i++) {
        seq_printf(s, "%d\t%s\n", task->pid, task->comm );
    }
    return 0;
}

/**
 * This structure gather "function" to manage the sequence
 *
 */
static struct seq_operations my_seq_ops = {
    .start = my_seq_start,
    .next  = my_seq_next,
    .stop  = my_seq_stop,
    .show  = my_seq_show
};

/**
 * This function is called when the /proc file is open.
 *
 */
static int my_open(struct inode *inode, struct file *file)
{
    return seq_open(file, &my_seq_ops);
};

/**
 * This structure gather "function" that manage the /proc file
 *
 */
static struct file_operations my_file_ops = {
    .owner   = THIS_MODULE,
    .open    = my_open,
    .read    = seq_read,
    .llseek  = seq_lseek,
    .release = seq_release
};


/**
 * This function is called when the module is loaded
 *
 */
int init_module(void)
{
    struct proc_dir_entry *entry;

    entry = proc_create(PROC_NAME, 0666, NULL, &my_file_ops);
    return 0;
}

/**
 * This function is called when the module is unloaded.
 *
 */
void cleanup_module(void)
{
    remove_proc_entry(PROC_NAME, NULL);
}
