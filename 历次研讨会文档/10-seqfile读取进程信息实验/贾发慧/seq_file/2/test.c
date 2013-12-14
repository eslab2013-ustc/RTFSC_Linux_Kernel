#include<linux/kernel.h>
#include<linux/module.h>
#include<linux/mutex.h>
#include<linux/proc_fs.h>
#include<linux/seq_file.h>
#include<unistd.h>
static struct mutex lock;
static struct list_head head;
static int My_jfhcount=1;
static int My_stop=1;
struct my_data {
        struct list_head list;
        int    value;
};static void add_one(void)
{
        struct my_data *data;
        mutex_lock(&lock);
        data = kmalloc(sizeof(*data), GFP_KERNEL);
        data->value=My_jfhcount;
        My_jfhcount=My_jfhcount+1;
        if (data != NULL)
                list_add(&data->list, &head);
        mutex_unlock(&lock);
}
static ssize_t _seq_write(struct file *file, const char __user * buffer,
                       size_t count, loff_t *ppos)
{
        add_one();
        return count;
}
static int _seq_show(struct seq_file *m, void *p)
{
        struct my_data *data = list_entry(p, struct my_data, list);
        seq_printf(m, "value: %d\n", data->value);
        return 0;
}
static void *_seq_start(struct seq_file *m, loff_t *pos)
{
        mutex_lock(&lock);
        return seq_list_start(&head, *pos);
}
static void *_seq_next(struct seq_file *m, void *p, loff_t *pos)
{
        return seq_list_next(p, &head, pos);
}
static void _seq_stop(struct seq_file *m, void *p)
{
        My_stop=My_stop+1;
        mutex_unlock(&lock);
        
}
static struct seq_operations _seq_ops = {
        .start  = _seq_start,
        .next   = _seq_next,
        .stop   = _seq_stop,
        .show   = _seq_show
};
static int _seq_open(struct inode *inode, struct file *file)
{
        return seq_open(file, &_seq_ops);
}
static struct file_operations _seq_fops = {
        .open           = _seq_open,
        .read           = seq_read,
        .write          = _seq_write,
        .llseek         = seq_lseek,
        .release        = seq_release
};
static void clean_all(struct list_head *head)
{
        struct my_data *data;
        while (!list_empty(head)) {
                data = list_entry(head->next, struct my_data, list);
                list_del(&data->list);
                kfree(data);
        }
}
static int __init init(void)
{
        struct proc_dir_entry *entry;
        int i=0;
        mutex_init(&lock);
        INIT_LIST_HEAD(&head);
        while(i<100000)
        {add_one();
         i=i+1;  
        }
        add_one();
        add_one();
        entry = create_proc_entry("my_data", S_IWUSR | S_IRUGO, NULL);
        if (entry == NULL) {
                clean_all(&head);
                return -ENOMEM;
        }
        entry->proc_fops = &_seq_fops;
        return 0;
}
static void __exit fini(void)
{
        remove_proc_entry("my_data", NULL);
printk(KERN_INFO "the number of the stop function exe is:%d\n",My_stop);
        clean_all(&head);
}
module_init(init);
module_exit(fini);
