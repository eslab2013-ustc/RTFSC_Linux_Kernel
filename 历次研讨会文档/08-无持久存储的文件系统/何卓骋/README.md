<!--Meta fname:proc-rw author:'farseer' theme:'moon' title:proc_fs_rw -->
<!--sec1-->
# proc数据项的读写操作

<!--sec2.1-->
## proc_create

<!--proc_create-->
<!--    |-->
<!--    v-->
<!--proc_create_data-->
<!--    |-->
<!--    ___> __proc_create-->
<!--    |-->
<!--    _____> proc_register-->
<!--               |-->
<!--               _____> proc_alloc_inum-->

![proc_create](https://raw.github.com/eslab2013-ustc/RTFSC_Linux_Kernel/master/%E5%8E%86%E6%AC%A1%E7%A0%94%E8%AE%A8%E4%BC%9A%E6%96%87%E6%A1%A3/08-%E6%97%A0%E6%8C%81%E4%B9%85%E5%AD%98%E5%82%A8%E7%9A%84%E6%96%87%E4%BB%B6%E7%B3%BB%E7%BB%9F/%E4%BD%95%E5%8D%93%E9%AA%8B/images/create.png)

linux/proc_fs.h

```c
static inline struct proc_dir_entry *proc_create(
    const char *name, umode_t mode, struct proc_dir_entry *parent,
    const struct file_operations *proc_fops)
{
    return proc_create_data(name, mode, parent, proc_fops, NULL);
}
```

<!--sec2.2-->
## proc_create_data

fs/proc/generic.c

```c
struct proc_dir_entry *proc_create_data(const char *name, umode_t mode,
                    struct proc_dir_entry *parent,
                    const struct file_operations *proc_fops,
                    void *data)
{
    struct proc_dir_entry *pde;
    if ((mode & S_IFMT) == 0)
        mode |= S_IFREG;

    if (!S_ISREG(mode)) {
        WARN_ON(1); /* use proc_mkdir() */
        return NULL;
    }

    if ((mode & S_IALLUGO) == 0)
        mode |= S_IRUGO;
    pde = __proc_create(&parent, name, mode, 1);
    if (!pde)
        goto out;
    pde->proc_fops = proc_fops;
    pde->data = data;
    if (proc_register(parent, pde) < 0)
        goto out_free;
    return pde;
out_free:
    kfree(pde);
out:
    return NULL;
}
```

<!--sec2.3-->
## __proc_create

fs/proc/generic.c

```c
static struct proc_dir_entry *__proc_create(struct proc_dir_entry **parent,
                      const char *name,
                      umode_t mode,
                      nlink_t nlink)
{
    struct proc_dir_entry *ent = NULL;
    const char *fn = name;
    unsigned int len;

    /* make sure name is valid */
    if (!name || !strlen(name))
        goto out;

    if (xlate_proc_name(name, parent, &fn) != 0)
        goto out;

    /* At this point there must not be any '/' characters beyond *fn */
    if (strchr(fn, '/'))
        goto out;

    len = strlen(fn);

    ent = kzalloc(sizeof(struct proc_dir_entry) + len + 1, GFP_KERNEL);
    if (!ent)
        goto out;

    memcpy(ent->name, fn, len + 1);
    ent->namelen = len;
    ent->mode = mode;
    ent->nlink = nlink;
    atomic_set(&ent->count, 1);
    spin_lock_init(&ent->pde_unload_lock);
    INIT_LIST_HEAD(&ent->pde_openers);
out:
    return ent;
}
```

<!--sec2.4-->
## proc_register

fs/proc/generic.c

```c
static int proc_register(struct proc_dir_entry * dir, struct proc_dir_entry * dp)
{
    struct proc_dir_entry *tmp;
    int ret;

    ret = proc_alloc_inum(&dp->low_ino);
    if (ret)
        return ret;

    if (S_ISDIR(dp->mode)) {
        dp->proc_fops = &proc_dir_operations;
        dp->proc_iops = &proc_dir_inode_operations;
        dir->nlink++;
    } else if (S_ISLNK(dp->mode)) {
        dp->proc_iops = &proc_link_inode_operations;
    } else if (S_ISREG(dp->mode)) {
        BUG_ON(dp->proc_fops == NULL);
        dp->proc_iops = &proc_file_inode_operations;
    } else {
        WARN_ON(1);
        return -EINVAL;
    }

    spin_lock(&proc_subdir_lock);

    for (tmp = dir->subdir; tmp; tmp = tmp->next)
        if (strcmp(tmp->name, dp->name) == 0) {
            WARN(1, "proc_dir_entry '%s/%s' already registered\n",
                dir->name, dp->name);
            break;
        }

    dp->next = dir->subdir;
    dp->parent = dir;
    dir->subdir = dp;
    spin_unlock(&proc_subdir_lock);

    return 0;
}
```

<!--sec3.1-->
## proc_file_operations

内核使用保存在proc_file_operations中的操作来读写常规proc数据项(reg)的内容。
该结构中的函数指针，所指向的目标函数如下：

fs/proc/inode.c

```c
static const struct file_operations proc_reg_file_ops = {
    .llseek     = proc_reg_llseek, // man lseek : move the read/write file offset
    .read       = proc_reg_read,
    .write      = proc_reg_write,
    .poll       = proc_reg_poll,  // man poll : input/output multiplexing
    .unlocked_ioctl = proc_reg_unlocked_ioctl, // man ioctl : control a STREAM device
#ifdef CONFIG_COMPAT
    .compat_ioctl   = proc_reg_compat_ioctl,
#endif
    .mmap       = proc_reg_mmap,
    .open       = proc_reg_open,
    .release    = proc_reg_release,
};
```

<!--sec3.2-->
### proc_reg_read

fs/proc/inode.c

```c
static ssize_t proc_reg_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    ssize_t (*read)(struct file *, char __user *, size_t, loff_t *);
    struct proc_dir_entry *pde = PDE(file_inode(file));
    ssize_t rv = -EIO;
    if (use_pde(pde)) {
        read = pde->proc_fops->read;
        if (read)
            rv = read(file, buf, count, ppos);
        unuse_pde(pde);
    }
    return rv;
}
```

<!--sec3.3-->
### proc_reg_write

fs/proc/inode.c

```c
static ssize_t proc_reg_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    ssize_t (*write)(struct file *, const char __user *, size_t, loff_t *);
    struct proc_dir_entry *pde = PDE(file_inode(file));
    ssize_t rv = -EIO;
    if (use_pde(pde)) {
        write = pde->proc_fops->write;
        if (write)
            rv = write(file, buf, count, ppos);
        unuse_pde(pde);
    }
    return rv;
}
```

<!--sec4.1-->
## read

读取数据的操作步骤：

* 分配一个内核内存页面，产生的数据将填充到页面中;
* 调用一个特定于文件的函数，向内核内存页面填充数据;
* 数据从内核空间复制到用户空间。

<!--sec4.2-->
## write

写入数据的操作步骤：

* 检查用户输入的长度（使用count参数确定），确保不超出所分配区域的长度。
* 数据从用户空间复制到分配的内核空间区域。
* 从字符串中提取出信息。（parsing）
* 根据接收到的用户信息，对该系统进行操作。

<!--sec5.1-->
## seq_file

fs/seq_file.c
linux/seq_file.h

* It provides a safer interface to the /proc filesystem than previous procfs methods
* It protects against overflow of the output buffer
* Easily handles procfs files that are larger than one page
* Provides methods for traversing a list of kernel items and iterating on that list
* seq_file operates by using "pull" methods, whereas the previous procfs methods pushed data into output buffers

```c
struct seq_file {
    char *buf;
    size_t size;
    size_t from;
    size_t count;
    loff_t index;
    loff_t read_pos;
    u64 version;
    struct mutex lock;
    const struct seq_operations *op;
    int poll_event;
#ifdef CONFIG_USER_NS
    struct user_namespace *user_ns;
#endif
    void *private;
};

struct seq_operations {
    void * (*start) (struct seq_file *m, loff_t *pos);
    void (*stop) (struct seq_file *m, void *v);
    void * (*next) (struct seq_file *m, void *v, loff_t *pos);
    int (*show) (struct seq_file *m, void *v);
};
```

<!--sec5.2-->
### seq_open/single_open

```c
int single_open(struct file *file, int (*show)(struct seq_file *, void *),
        void *data)
{
    struct seq_operations *op = kmalloc(sizeof(*op), GFP_KERNEL);
    int res = -ENOMEM;

    if (op) {
        op->start = single_start;
        op->next = single_next;
        op->stop = single_stop;
        op->show = show;
        res = seq_open(file, op);
        if (!res)
            ((struct seq_file *)file->private_data)->private = data;
        else
            kfree(op);
    }
    return res;
}

int seq_open(struct file *file, const struct seq_operations *op)
{
    struct seq_file *p = file->private_data;

    if (!p) {
        p = kmalloc(sizeof(*p), GFP_KERNEL);
        if (!p)
            return -ENOMEM;
        file->private_data = p;
    }
    memset(p, 0, sizeof(*p));
    mutex_init(&p->lock);
    p->op = op;
#ifdef CONFIG_USER_NS
    p->user_ns = file->f_cred->user_ns;
#endif

    /*
     * Wrappers around seq_open(e.g. swaps_open) need to be
     * aware of this. If they set f_version themselves, they
     * should call seq_open first and then set f_version.
     */
    file->f_version = 0;

    /*
     * seq_files support lseek() and pread().  They do not implement
     * write() at all, but we clear FMODE_PWRITE here for historical
     * reasons.
     *
     * If a client of seq_files a) implements file.write() and b) wishes to
     * support pwrite() then that client will need to implement its own
     * file.open() which calls seq_open() and then sets FMODE_PWRITE.
     */
    file->f_mode &= ~FMODE_PWRITE;
    return 0;
}
```

<!--sec5.3-->
### seq_read/seq_write

```c
ssize_t seq_read(struct file *file, char __user *buf, size_t size, loff_t *ppos)
{
    struct seq_file *m = file->private_data;
    size_t copied = 0;
    loff_t pos;
    size_t n;
    void *p;
    int err = 0;

    mutex_lock(&m->lock);

    /*
     * seq_file->op->..m_start/m_stop/m_next may do special actions
     * or optimisations based on the file->f_version, so we want to
     * pass the file->f_version to those methods.
     *
     * seq_file->version is just copy of f_version, and seq_file
     * methods can treat it simply as file version.
     * It is copied in first and copied out after all operations.
     * It is convenient to have it as  part of structure to avoid the
     * need of passing another argument to all the seq_file methods.
     */
    m->version = file->f_version;

    /* Don't assume *ppos is where we left it */
    if (unlikely(*ppos != m->read_pos)) {
        while ((err = traverse(m, *ppos)) == -EAGAIN)
            ;
        if (err) {
            /* With prejudice... */
            m->read_pos = 0;
            m->version = 0;
            m->index = 0;
            m->count = 0;
            goto Done;
        } else {
            m->read_pos = *ppos;
        }
    }

    /* grab buffer if we didn't have one */
    if (!m->buf) {
        m->buf = kmalloc(m->size = PAGE_SIZE, GFP_KERNEL);
        if (!m->buf)
            goto Enomem;
    }
    /* if not empty - flush it first */
    if (m->count) {
        n = min(m->count, size);
        err = copy_to_user(buf, m->buf + m->from, n);
        if (err)
            goto Efault;
        m->count -= n;
        m->from += n;
        size -= n;
        buf += n;
        copied += n;
        if (!m->count)
            m->index++;
        if (!size)
            goto Done;
    }
    /* we need at least one record in buffer */
    pos = m->index;
    p = m->op->start(m, &pos);
    while (1) {
        err = PTR_ERR(p);
        if (!p || IS_ERR(p))
            break;
        err = m->op->show(m, p);
        if (err < 0)
            break;
        if (unlikely(err))
            m->count = 0;
        if (unlikely(!m->count)) {
            p = m->op->next(m, p, &pos);
            m->index = pos;
            continue;
        }
        if (m->count < m->size)
            goto Fill;
        m->op->stop(m, p);
        kfree(m->buf);
        m->buf = kmalloc(m->size <<= 1, GFP_KERNEL);
        if (!m->buf)
            goto Enomem;
        m->count = 0;
        m->version = 0;
        pos = m->index;
        p = m->op->start(m, &pos);
    }
    m->op->stop(m, p);
    m->count = 0;
    goto Done;
Fill:
    /* they want more? let's try to get some more */
    while (m->count < size) {
        size_t offs = m->count;
        loff_t next = pos;
        p = m->op->next(m, p, &next);
        if (!p || IS_ERR(p)) {
            err = PTR_ERR(p);
            break;
        }
        err = m->op->show(m, p);
        if (seq_overflow(m) || err) {
            m->count = offs;
            if (likely(err <= 0))
                break;
        }
        pos = next;
    }
    m->op->stop(m, p);
    n = min(m->count, size);
    err = copy_to_user(buf, m->buf, n);
    if (err)
        goto Efault;
    copied += n;
    m->count -= n;
    if (m->count)
        m->from = n;
    else
        pos++;
    m->index = pos;
Done:
    if (!copied)
        copied = err;
    else {
        *ppos += copied;
        m->read_pos += copied;
    }
    file->f_version = m->version;
    mutex_unlock(&m->lock);
    return copied;
Enomem:
    err = -ENOMEM;
    goto Done;
Efault:
    err = -EFAULT;
    goto Done;
}

int seq_write(struct seq_file *seq, const void *data, size_t len)
{
    if (seq->count + len < seq->size) {
        memcpy(seq->buf + seq->count, data, len);
        seq->count += len;
        return 0;
    }
    seq_set_overflow(seq);
    return -1;
}
```

<!--sec9-->
## code

```c
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <asm/uaccess.h>

char *msg = "Hello world!";

static int HelloProc_proc_show(struct seq_file *m, void *v)
{
    seq_printf(m, "%s\n", msg);
    return 0;
}

static int HelloProc_proc_open(struct inode *inode, struct file *file)
{
    return single_open(file, HelloProc_proc_show, NULL);
}

static int HelloProc_proc_write(struct file *filp, char *buf, size_t count, loff_t *offp)
{
    copy_from_user(msg, buf, count);
    return count;
}

static const struct file_operations HelloProc_proc_fops = {
    .open       = HelloProc_proc_open,
    .read       = seq_read,
    .write      = HelloProc_proc_write,
    .llseek     = seq_lseek,
    .release    = single_release,
};

int __init proc_HelloProc_init(void)
{
    proc_create("HelloProc", 0666, NULL, &HelloProc_proc_fops);
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

```

### Makefile

```make
obj-m := HelloProc.o
KERNELDIR := /lib64/modules/$(shell uname -r)/build
PWD := $(shell pwd)

default:
    $(MAKE) -C $(KERNELDIR) M=$(PWD) modules

clean:
    rm -rf *.o *~ core .depend *.order *.symvers .*.cmd *.ko *.mod.c .tmp_versions
```

<!--sec10-->
## reference

* http://tuxthink.blogspot.com/2013/10/creating-read-only-proc-entry-in-kernel.html
* http://tuxthink.blogspot.com/2013/10/creating-read-write-proc-entry-in.html
