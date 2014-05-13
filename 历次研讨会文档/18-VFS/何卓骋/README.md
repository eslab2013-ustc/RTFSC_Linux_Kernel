# File Operations

* VFS 层提供了抽象的操作，来连接文件对象和底层机制 -> 通用性
* 每个文件实例对应一个struct file_operations 指针

## file_operations

* `struct module *owner`;  owner 只有在filesystem被作为module加载，而不是编译进内核的时候使用，此时该指针指向对应的module的数据结构。

| read/write               | 读写。参数:文件描述符，缓冲区，偏移量，字节数。                                                 |
| aio_read/aio_write       | 异步读写操作                                                                                    |
| open                     | 关联文件对象和inode                                                                             |
| release                  | 引用计数到0时调用，释放内存和cache                                                              |
| mmap                     | 使用虚拟进程空间进行文件映射                                                                    |
| readdir                  | 读取文件目录                                                                                    |
| ioctl                    | 于硬件交互，当需要向硬件发出控制指令时调用                                                      |
| poll                     | 伴随poll和select系统调用，同步多重I/O，设置一个计时器，如果始终没有可读数据，则继续而不是阻塞。 |
| flush                    | 文件描述符关闭时调用，伴随着引用计数减少，通常用于网络文件系统确认传输。                        |
| fsync                    | 伴随fsync和fdatasync系统调用来初始化内存和存储媒介之间的同步                                    |
| fasync                   | 信号控制输入和输出，文件对象的修改通过信号来告知进程                                            |
| readv/writev             | 向量读写                                                                                        |
| lock                     | 加锁，同步多进程同时对文件的访问                                                                |
| revalidate               | 用于网络文件系统，当发生media change之后，确保远程数据的一致                                    |
| check_media_change       | 检查从上次访问以来是否发生了media change                                                        |
| sendfile                 | 在两个文件描述符之间交换数据（通过sendfile系统调用），同时用于简单高效的网络数据传输            |
| splice_read/splice_write | 用于在管道和文件之间传输数据                                                                    |

* 不同类型的操作声明。例如:
{% highlight c %}
    const struct file_operations def_blk_fops = {
        .open = blkdev_open,
        .release = blkdev_close,
        .llseek = block_llseek,
        .read = do_sync_read,
        .write = do_sync_write,
        .aio_read = generic_file_aio_read,
        .aio_write = generic_file_aio_write_nolock,
        .mmap = generic_file_mmap,
        .fsync = block_fsync,
        .unlocked_ioctl = block_ioctl,
        .splice_read = generic_file_splice_read,
        .splice_write = generic_file_splice_write,
    };
{% endhighlight %}

### Directory information

* task_struct 中包含一个指向fs_struct的指针

{% highlight c %}
    struct fs_struct {
        atomic_t count;
        int umask;
        struct dentry * root, * pwd, * altroot;
        struct vfsmount * rootmnt, * pwdmnt, * altrootmnt;
    }
{% endhighlight %}

* umask 表示新建文件的默认permission。可以通过命令umask更改，调用同名的系统调用
* dentry 指向目录名，vfsmount表示了挂载文件系统。
* 3对dentry和VFS 挂载元素:
    * root/rootmnt 根目录和相关进程的文件系统，通常是系统的根目录 /。可以通过chroot修改
    * pwd/pwdmnt 当前目录和文件系统的VFS挂载数据结构。当如下操作 cd /mnt; cd floppy 发生时。
        1. cd /mnt 改变pwd，不改变pwdmnt，因为始终处于root目录的领域。
        2. cd floopy 两者都改变，因为进入了一个新的文件系统的领域。
    * altroot/altrootmnt 通常用于模拟非linux系统。

### VFS namespace

* 是一个所有挂载了的文件系统的集合
* forked 或者 cloned的进程集成了父进程的命名空间。如果CLONE_NEWNS被置位，则创建新的ns，如果新ns被修改，不会影响属于不同ns的进程。
* task_struct 中包含一个元素nsproxy，用于处理ns
{% highlight c %}
    struct nsproxy {
        ...
        struct mnt_namespace *mnt_ns;
        ...
    };

    struct mnt_namespace {
        atomic_t count;
        struct vsmount * root;
        struct list_head list;
    }
{% endhighlight %}

* count 计数，多少个进程在使用该ns
* root 指向root目录的vfsmount实例
* list 双向链表(包含所有vfsmount实例，用其mnt_list元素连接)的起始

ns操作例如mount和umount并不对内核全局数据结构进行，而是操作当前进程所使用的ns，这样的操作会影响所有使用同一个ns实例的进程。

### Directory Entry cache

* 得到了存储器媒介的某个区域，要寻找其对应的被某个文件名关联的inode，要很长的时间，即使设备数据已经存在与页表cache中
* linux使用 dentry cache来解决这个问题，cache被建立在struct dentry中
* 一旦VFS，伴随具体的文件系统实现，读了某个目录或者文件的数据，一个dentry实例便被创建以便于将数据进行cache

{% highlight c %}
    struct dentry {
        /* RCU lookup touched fields */
        unsigned int d_flags;		/* protected by d_lock */
        seqcount_t d_seq;		/* per dentry seqlock */
        struct hlist_bl_node d_hash;	/* lookup hash list */
        struct dentry *d_parent;	/* parent directory */
        struct qstr d_name;
        struct inode *d_inode;		/* Where the name belongs to - NULL is
                        * negative */
        unsigned char d_iname[DNAME_INLINE_LEN];	/* small names */

        /* Ref lookup also touches following */
        struct lockref d_lockref;	/* per-dentry lock and refcount */
        const struct dentry_operations *d_op;
        struct super_block *d_sb;	/* The root of the dentry tree */
        unsigned long d_time;		/* used by d_revalidate */
        void *d_fsdata;			/* fs-specific data */

        struct list_head d_lru;		/* LRU list */
        /*
        * d_child and d_rcu can share memory
        */
        union {
            struct list_head d_child;	/* child of parent list */
            struct rcu_head d_rcu;
        } d_u;
        struct list_head d_subdirs;	/* our children */
        struct hlist_node d_alias;	/* inode alias list */
    };
{% endhighlight %}

* dentry 的实例形成一个网络来映射文件系统的结构。与给定dentry所有相关的文件和子目录都被包括在d_subdirs（也是dentry实例）列表中。
* 文件系统的拓扑仍然没有完全被映射，因为dentry cache只包含了其中的一小部分，最常使用的文件和目录被保存在内存。
* dentry数据结构的主要用处就是来连接filename和其对应的inode，3个关键成员:
    * d_inode 指向inode实例的指针，NUll说明文件名不存在
    * d_name 文件的名称，gstr: 内核的字符串封装，不但存储了字符串，还包括其长度，hash值。d_name 并不包括绝对路径，只有最后一个文件名。
    * d_iname 如果文件名很短，用d_iname 来加速访问。
* d_flags ， DCACHE_DISCONNECTED 表示dentry没有被连接到超块的dentry tree上。DCACHE_UNHASHED表示dentry实例没有被包含在任何一个inode的hash表中。
* d_parent 指向父目录的dentry实例。
* d_mounted 1 如果对应的dentry代表了一个挂载点，否则为0
* d_alias 连接了相同文件的所有dentry
* d_op 指向dentry对象的操作函数指针构成的数据结构。
* s_sb 指向dentry 对象所处的superblock实例

* 所有内存中的dentry实例存储在一张hash表 dentry_hashtable 中，d_hash用于处理hash冲突。
* dentry_unused，包含了usage count(d_count)为0的dentry的列表头。

### Cache Organization

* dentry 数据结构使得处理文件系统结构更加简单，提升了系统的性能。通过最小化与底层文件系统实现间的通信来加速VFS。
* 每次对底层实现的请求都伴随着一个新dentry对象的创建(用于保存请求结果)，这些对象被存储在cache中。
* dentry对象在内存中的管理:
    * dentry_hashtable 包含所有dentry对象
    * LRU 链表。用dentry中的d_lru元素连接
* d_hash 函数被用于确定dentry对象的hash索引。
* dentry 被放置到LRU链表，当其计数到0，新加入的entry永远置于表头的位置
* prune_dcache 函数被时不时调用，用于删除对象，释放内存。

### Dentry Operations

* dentry_operations 结构包含了各种dentry操作的函数指针，如下:
{% highlight c %}
    struct dentry_operations {
        int (*d_revalidate)(struct dentry *, struct nameidata *);
        int (*d_hash) (struct dentry *, struct qstr *);
        int (*d_compare) (struct dentry *, struct qstr *, struct qstr *);
        int (*d_delete)(struct dentry *);
        void (*d_release)(struct dentry *);
        void (*d_iput)(struct dentry *, struct inode *);
        char *(*d_dname)(struct dentry *, char *, int);
    };
{% endhighlight %}

* d_iput 释放不用了的dentry的inode
* d_release 在dentry被最终删除之前调用，
* d_hash 计算hash表中用于存放dentry对象的位置
* d_compare 比较两个dentry的名称，
* d_revalidate 与前文描述的revalidate类似。
* 如果函数指针为NUll，由VFS的缺省函数填充。

### Standard functions

| dget           | 当dentry实例被内核的某部分需求时调用，引用计数加1                                                                   |
| dput           | 与dget相对，引用减1，如果降至0，则调用dentry_operations->d_delete 来删除。从全局hash表中删除(d_drop)，放置到LRU上。 |
| d_drop         | unhash                                                                                                              |
| d_delete       | d_drop, 如果只有一个user关联，dentry_iput 减1，d_delete 一般在dput之前调用。                                        |
| d_instantiate  | 关联inode和dentry                                                                                                   |
| d_add          | d_instantiate, 加入全局hash表                                                                                       |
| d_alloc        | 为新的dentry分配内存，关联父子dentry                                                                                |
| d_alloc_anon   | 只分配内存，不关联父子                                                                                              |
| d_splice_alias | 将disconnected dentry拼接到dentry tree上                                                                            |
| d_lookup       | 在给定的dentry下，查找给定的名称对应的dentry                                                                        |
