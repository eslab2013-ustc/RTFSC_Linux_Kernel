<!--Meta fname:sem author:'farseer' theme:'solarized' title:sem_data_structure -->
<!--sec1-->
## 信号量机制 -- 数据结构

![ds](https://raw.github.com/eslab2013-ustc/RTFSC_Linux_Kernel/master/%E5%8E%86%E6%AC%A1%E7%A0%94%E8%AE%A8%E4%BC%9A%E6%96%87%E6%A1%A3/06-%E8%BF%9B%E7%A8%8B%E9%97%B4%E9%80%9A%E4%BF%A1/%E4%BD%95%E5%8D%93%E9%AA%8B/images/data-structure.png)

* [ids](#/2)
* [kern_ipc_perm](#/3)
* [sem_queue](#/4)
* [sem_array](#/5)


<!--sec2-->
### namespace

* 没有层次关系
* 给定的进程属于**task_struct->nsproxy->ipc_ns**指向的命名空间。
* 初始默认命名空间通过**ipc_namespace** 的静态实例**init_ipc_ns**实现。
* 资源限制:参见**msgget, shmget, semget** 的manpage。

&lt;ipc.h&gt;

``` c
struct ipc_namespace {
    atomic_t    count;
    struct ipc_ids  ids[3]; // 3种ipc机制
    ...
    /* 资源限制 */
}
```

<!--sec3-->
### ids

ipc/util.h

```c
struct ipc_ids {
    int in_use;                // 当前使用中ipc对象的数目
    unsigned short seq;        // 序号
    unsigned short seq_max;    // 最大序号
    struct rw_semaphore rwsem; // 内核信号量，保护了信号量值的数据结构
    struct idr ipcs_idr;       // 用于将ID关联到指向对应的kern_ipc_perm实例的指针
    int next_id;
};

```

* 序号不等同于id。
* 内核通过id来标识IPC对象，id按资源类型管理。
* 每次创建新的ipc对象时，序号加1，到达seq_max则清零。
* 用户层id由s * SEQ_MULTIPLIER+i 给出，s为当前序号，i是内核内部id。SEQ_MULTIPLIER 设置为ipc对象的上限。
* 如果重用了内核id，任然会产生不同的用户空间id

[image](#/0)

<!--sec4-->
### kern_ipc_perm

```c
struct kern_ipc_perm
{
    spinlock_t  lock;
    int     deleted;
    int     id;          // 内核内部id
    key_t       key;     // 用户程序用来标识信号量的魔数
    kuid_t      uid;     // 所有者id
    kgid_t      gid;     // 所有者组id
    kuid_t      cuid;    // 产生信号量的进程的用户id
    kgid_t      cgid;    // ..................组id
    umode_t     mode;    // 位掩码，指定访问权限
    unsigned long   seq; // 序号，分配ipc对象时使用
    void        *security;
};

...

struct task_struct
{
...
#ifdef CONFIG_SYSVIPC
/* ipc stuff */
    struct sysv_sem sysvsem;
#endif
...
};

struct sysv_sem {
    struct sem_undo_list *undo_list; // 用于撤销信号量
};

```

* kern_ipc_perm 标识了该ipc对象的权限。
* 不仅可用于信号量，还可用于其他的ipc机制（结构体头部）。
* 如果进程在修改信号量之后崩溃。undo_list 可用于将信号量的状态恢复到修改之前。保持信号量一致，防止死锁。

[image](#/0)

<!--sec5-->
### sem_queue

&lt;sem.h&gt;
```c
struct sem_queue {
    struct sem_queue *  next;    /*next entry in the queue */
    struct sem_queue ** prev;    /*previous entry in the queue, *(q->prev) == q */
    struct task_struct* sleeper; /*this process */
    struct sem_undo *   undo;    /*undo structure */
    int                 pid;     /*process id of requesting process */
    int                 status;  /*completion status of operation */
    struct sem_array *  sma;     /*semaphore array for operations */
    int                 id;      /*internal sem id */
    struct sembuf *     sops;    /*array of pending operations */
    int                 nsops;   /*number of operations */
    int                 alter;   /*does the operation alter the array? */
};

...

struct sembuf {
    unsigned short sem_num; // 信号量在数组中的索引
    short sem_op;           // 信号量操作
    short sem_flg;          // 操作标志
};
```

* 对于每个信号量，都有一个队列管理与信号量相关的所有睡眠进程。
* sem_queue 用于将信号量与睡眠进程（该进程想要执行信号量操作，但是当前不允许）关联起来。
* sembuf 操作记录

[image](#/0)

<!--sec6-->
&lt;sem.h&gt;
### sem_array & sem

```c
struct sem_array {
    struct kern_ipc_perm sem_perm;           /* permissions .. see ipc.h */
    time_t               sem_otime;          /* last semop time */ // 上次操作时间
    time_t               sem_ctime;          /* last change time */
    struct sem           *sem_base;          /* ptr to first semaphore in array */
    struct sem_queue     *sem_pending;       /* pending operations to be processed */
    struct sem_queue     **sem_pending_last; /* last pending operation */
    struct sem_undo      *undo;              /* undo requests on this array */
    unsigned long        sem_nsems;          /* no. of semaphores in array */
};

...

struct sem {
    int semval;    // 当前值
    int sempid;    // 上次操作进程的pid
};
```

* 每个信号量集合，都对应于一个sem_array, 用于管理集合中所有的信号量。
* sem 用于保存各个信号量的值。

[image](#/0)

