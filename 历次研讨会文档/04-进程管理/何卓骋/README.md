<!--Meta fname:scheduler author:'farseer' theme:'solarized' title:scheduler_intro-->
<!--sec1-->
## 调度器简介

###### 何卓骋

<!--sec2-->
### 调度器

* 在程序之间共享cpu时间，创造并行执行的错觉。
* 分两个不同部分：一个涉及调度策略，一个涉及上下文切换。
* 按照所能分配的计算能力，向系统中的每个进程提供最大的公正性。

### linux调度器的特点

* 不需要传统的时间片概念。
* 只考虑进程等待的时间。等待时间最长的进程被调度。

<!--sec3-->
### 真相
![scheduler](https://raw.github.com/eslab2013-ustc/RTFSC_Linux_Kernel/master/%E5%8E%86%E6%AC%A1%E7%A0%94%E8%AE%A8%E4%BC%9A%E6%96%87%E6%A1%A3/04-%E8%BF%9B%E7%A8%8B%E7%AE%A1%E7%90%86/%E4%BD%95%E5%8D%93%E9%AA%8B/images/scheduler1.png)

<!--sec4.1-->
### 红黑树
* 可运行进程按等待时间在一个红黑树中排序。
* 平衡树的一种
* 查找，插入，删除节点的时间复杂度为O(logn)
* 具体实现参见算法导论三版第13章

<!--sec4.2-->
### 虚拟时钟
* **fair_clock = cpu_clock/process_number** 反映了cfs下每个进程会得到的cpu时间
* wait_runtime 度量了实际系统不足造成的不公平。在进程允许运行时，**wait_runtime = wait_runtime - runtime**
* 在红黑树中排序的关键字: **fair_clock - wait_runtime**

<!--sec4.3-->
### 虚拟时钟2
* sched_entity中的vruntime的值
* 对于nice值0的进程来说虚拟时间跟物理时间是相等的
* 对于其他的进程 **虚拟时间 = 物理时间* NICE_0_LOAD/load.weight**
* min_vruntime 单调递增，跟踪记录队列上所有进程的最小虚拟运行时间
* 红黑树中的排序键值：**se->vruntime - cfs_rq->min_vruntime** , 新版中则直接比较vruntime
* 优先级越高，虚拟时间增加越慢，在红黑树上往右移动速度越慢

<!--sec5-->
### 组件

![scheduler](https://raw.github.com/eslab2013-ustc/RTFSC_Linux_Kernel/master/%E5%8E%86%E6%AC%A1%E7%A0%94%E8%AE%A8%E4%BC%9A%E6%96%87%E6%A1%A3/04-%E8%BF%9B%E7%A8%8B%E7%AE%A1%E7%90%86/%E4%BD%95%E5%8D%93%E9%AA%8B/images/scheduler2.png)

* 通用调度器（主调度器+周期性调度器）：一个分配器，与其他两个组件交互。
* 调度类用于判断运行哪个进程。
* 选中进程之后，执行底层任务切换。

<!--sec6.1-->
## 数据结构
### task_struct
&lt;sched.h&gt;
``` c
struct task_struct {
    ...
    int prio, static_prio, normal_prio;    // 优先级
    unsigned int rt_priority;              // 实时进程优先级 0-99 值大优先级高
    const struct sched_class *sched_class; // 调度器类
    struct sched_entity se;                // 可调度cfs实体
    struct sched_rt_entity rt;             // 可调度rt实体
    unsigned int policy;                   // 调度策略
    cpumask_t cpus_allowed;                // 限制进程运行在哪些cpu上
    // unsigned int time_slice;               // 进程可用的cpu剩余时间段（RT）
    // struct list_head run_list;             // 运行表的表头（RT）
    ...
}

```
* 新版本中task_struct包涵一个sched_entity，一个sched_rt_entity。time_slice，run_list等则被封装在sched_rt_entity内

<!--sec6.2-->
* priority
    * prio/normal_prio 动态优先级，static_prio 静态（启动时分配，可用nice和sched_setscheduler系统调用修改）。
    * normal_prio 表示基于进程的静态优先级和调度策略计算出的优先级。进程分支时，子进程会继承normal_prio。
* policy

| SCHED_NORMAL | CFS | 普通进程                                        |
| ------------ | --- | ----------------------------------------------- |
| SCHED_BATCH  | CFS | 非交互，cpu使用密集的批处理进程, 不抢占其他进程 |
| SCHED_IDLE   | CFS | 次要进程                                        |
| SCHED_RR     | RT  | 循环                                            |
| SCHED_FIFO   | RT  | 先入先出                                        |

<!--sec6.3-->
### sched_class
&lt;sched.h&gt;
``` c
struct sched_class {
    const struct sched_class *next;
// 添加进程（睡眠状态->可运行状态）
    void (*enqueue_task) (struct rq *rq, struct task_struct *p, int wakeup);
// 去除进程（可运行->不可运行）
    void (*dequeue_task) (struct rq *rq, struct task_struct *p, int sleep);
// 放弃对处理器的控制权（sched_yield系统调用）
    void (*yield_task) (struct rq *rq);
// 用新唤醒的进程来抢占当前（例如wake_up_new_task时）
    void (*check_preempt_curr) (struct rq *rq, struct task_struct *p);
// 选择下一个将运行的进程
    struct task_struct * (*pick_next_task) (struct rq *rq);
// 用另一个进程代替当前进程之前被调用
    void (*put_prev_task) (struct rq *rq, struct task_struct *p);
// 调度策略变化时被调用
    void (*set_curr_task) (struct rq *rq);
// 由周期性调度器调用
    void (*task_tick) (struct rq *rq, struct task_struct *p);
// 新进程创建后调用
    void (*task_new) (struct rq *rq, struct task_struct *p);
};
```

<!--sec6.4-->
### rq
kernel/sched.c
``` c
struct rq {
    unsigned long nr_running;                 // 可运行的进程数
    #define CPU_LOAD_IDX_MAX 5                // 最大记录index
    unsigned long cpu_load[CPU_LOAD_IDX_MAX]; // 跟踪此前负荷
    ...
    struct load_weight load;                  // 当前负荷
    struct cfs_rq cfs;                        // 子就绪队列 用于CFS
    struct rt_rq rt;                          // 子就绪队列 用于RT
    struct task_struct *curr, *idle;          // 当前运行的进程，空闲进程
    u64 clock;                                // 物理时钟
    ...
};
```

* 各个cpu都有各自的rq
* 每个活动进程只能出现在一个rq上
* 每次调用周期性调度器时会更新clock，调用**update_rq_clock** 也能更新clock值
* 所有的rq都在runqueues数组中

``` c
#define cpu_rq(cpu)   (&per_cpu(runqueues, (cpu))) // 指定cpu的rq
#define this_rq()     (&__get_cpu_var(runqueues))  // 当前cpu的rq
#define task_rq(p)    cpu_rq(task_cpu(p))          // 指定task的rq
#define cpu_curr(cpu) (cpu_rq(cpu)->curr)          // 指定cpu的curr task
```

<!--sec6.5-->
### sched_entity
&lt;sched.h&gt;
``` c
struct sched_entity {
    struct load_weight load;   // 用于负载均衡
    struct rb_node run_node;   // 红黑树节点
    unsigned int on_rq;        // 是否在rq上
    u64 exec_start;            // 本次调用起始时刻
    u64 sum_exec_runtime;      // 总执行时间(物理时间)
    u64 vruntime;              // 进程执行期间虚拟时钟上流逝的时间
    u64 prev_sum_exec_runtime; // 保存sum_exec_runtime
    ...                        // 一些统计量
#ifdef CONFIG_FAIR_GROUP_SCHED
    struct sched_entity *parent;
    /* rq on which this entity is (to be) queued: */
    struct cfs_rq       *cfs_rq;
    /* rq "owned" by this entity/group: */
    struct cfs_rq       *my_q;
#endif

}

```

* 进程是可调度实体，但是可调度实体不一定是进程（便于实现调度组）
* 时间相关的变量由**update_curr**更新
* 可运行进程多于一个时,如果**sum_exec_runtime - prev_sum_exec_runtime**大于期望值,则由**resched_stask**发出重调度请求

<!--sec6.6-->
### sched_rt_entity
&lt;sched.h&gt;
``` c
struct sched_rt_entity {
    struct list_head run_list;
    unsigned long timeout;
    unsigned int time_slice;
    int nr_cpus_allowed;

    struct sched_rt_entity *back;
#ifdef CONFIG_RT_GROUP_SCHED
    struct sched_rt_entity  *parent;
    /* rq on which this entity is (to be) queued: */
    struct rt_rq        *rt_rq;
    /* rq "owned" by this entity/group: */
    struct rt_rq        *my_q;
#endif
};

```

<!--sec7.1-->
## 处理优先级
### 优先级的内核表示

![scheduler](https://raw.github.com/eslab2013-ustc/RTFSC_Linux_Kernel/master/%E5%8E%86%E6%AC%A1%E7%A0%94%E8%AE%A8%E4%BC%9A%E6%96%87%E6%A1%A3/04-%E8%BF%9B%E7%A8%8B%E7%AE%A1%E7%90%86/%E4%BD%95%E5%8D%93%E9%AA%8B/images/scheduler3.png)

<!--sec7.2-->
### nice

``` man
NAME
    nice - run a program with modified scheduling priority

SYNOPSIS
    nice [OPTION] [COMMAND [ARG]...]

DESCRIPTION
    Run COMMAND with an adjusted niceness, which affects process scheduling.  With no COMMAND, print the current niceness.  Niceness values range from -20 (most favor‐
    able to the process) to 19 (least favorable to the process).
```

### setpriority
``` man
NAME
    getpriority, setpriority - get/set program scheduling priority

SYNOPSIS
    #include <sys/time.h>
    #include <sys/resource.h>

    int getpriority(int which, int who);
    int setpriority(int which, int who, int prio);

DESCRIPTION
    The  scheduling  priority  of the process, process group, or user, as indicated by which and who is obtained with the getpriority() call and set with the setprior‐
    ity() call.

    The value which is one of PRIO_PROCESS, PRIO_PGRP, or PRIO_USER, and who is interpreted relative to which (a process identifier  for  PRIO_PROCESS,  process  group
    identifier for PRIO_PGRP, and a user ID for PRIO_USER).  A zero value for who denotes (respectively) the calling process, the process group of the calling process,
    or the real user ID of the calling process.  Prio is a value in the range -20 to 19 (but see the Notes below).  The default priority is 0; lower  priorities  cause
    more favorable scheduling.

    The getpriority() call returns the highest priority (lowest numerical value) enjoyed by any of the specified processes.  The setpriority() call sets the priorities
    of all of the specified processes to the specified value.  Only the superuser may lower priorities.

RETURN VALUE
    Since getpriority() can legitimately return the value -1, it is necessary to clear the external variable errno prior to the call, then check it afterward to deter‐
    mine if -1 is an error or a legitimate value.  The setpriority() call returns 0 if there is no error, or -1 if there is.
```

<!--sec7.3-->
### 相关宏
&lt;sched.h&gt;
``` c
#define MAX_USER_RT_PRIO 100
#define MAX_RT_PRIO      MAX_USER_RT_PRIO
#define MAX_PRIO         (MAX_RT_PRIO + 40)
#define DEFAULT_PRIO     (MAX_RT_PRIO + 20)

```

kernel/sched.c

``` c
#define NICE_TO_PRIO(nice) (MAX_RT_PRIO + (nice) + 20)
#define PRIO_TO_NICE(prio) ((prio) - MAX_RT_PRIO - 20)
#define TASK_NICE(p)       PRIO_TO_NICE((p)->static_prio)

```

<!--sec7.4-->
### 计算优先级
kernel/sched.c
``` c
static int effective_prio(struct task_struct *p)
{
    p->normal_prio = normal_prio(p);
    /*
    * If we are RT tasks or we were boosted to RT priority,
    * keep the priority unchanged. Otherwise, update priority
    * to the normal priority:
    */
    if (!rt_prio(p->prio)) // 根据优先级判断是否为实时进程
        return p->normal_prio;
    return p->prio;
}
...
static inline int normal_prio(struct task_struct *p)
{
    int prio;
    if (task_has_rt_policy(p))
        prio = MAX_RT_PRIO-1 - p->rt_priority;
    else
        prio = __normal_prio(p);
    return prio;
}
...
static inline int __normal_prio(struct task_struct *p)
{
    return p->static_prio;
}
```

| Task type          | static_prio | normal_prio    | prio        |
| ------------------ | ----------- | -------------- | ----------- |
| 普通               | static_prio | static_prio    | static_prio |
| 优先级提高的非实时 | static_prio | static_prio    | prio不变    |
| 实时               | static_prio | 99-rt_priority | prio不变    |

* 使用RT-Mutex 会导致情况2的发生
* 分支时，子进程的静态优先级继承自父进程，动态优先级则设置为父进程的普通优先级（确保了RT-Mutex引起的优先级提高不会传递到子进程）

<!--sec8.1-->
### 计算负荷权重
* 进程的重要性还需考虑task->se.load的负荷权重
* set_load_weight 负责根据进程类型和静态优先级计算负荷权重

&lt;sched.h&gt;
``` c
struct load_weight {
    unsigned long weight, inv_weight; // inv_weight意义貌似有变
}
```
#### 优先级/权重 转换表

kernel/sched.c
``` c
static const int prio_to_weight[40] = {
 /* -20 */     88761,     71755,     56483,     46273,     36291,
 /* -15 */     29154,     23254,     18705,     14949,     11916,
 /* -10 */      9548,      7620,      6100,      4904,      3906,
 /*  -5 */      3121,      2501,      1991,      1586,      1277,
 /*   0 */      1024,       820,       655,       526,       423,
 /*   5 */       335,       272,       215,       172,       137,
 /*  10 */       110,        87,        70,        56,        45,
 /*  15 */        36,        29,        23,        18,        15,
};
```

* 乘数因子1.25

<!--sec8.2-->
### 设置权重
kernel/sched.c
``` c
#define WEIGHT_IDLEPRIO 2
#define WMULT_IDLEPRIO  (1 << 31)
static void set_load_weight(struct task_struct *p)
{
    /* if (task_has_rt_policy(p)) { */
    /*     p->se.load.weight = prio_to_weight[0] * 2;          // RT进程的权重是nice值-20的权重的两倍 */
    /*     p->se.load.inv_weight = prio_to_wmult[0] >> 1; */
    /*     return; */
    /* } */
    /*
    * SCHED_IDLE tasks get minimal weight:
    */
    /* if (p->policy == SCHED_IDLE) { */
    /*     p->se.load.weight = WEIGHT_IDLEPRIO; */
    /*     p->se.load.inv_weight = WMULT_IDLEPRIO; */
    /*     return; */
    /* } */
    /* p->se.load.weight = prio_to_weight[p->static_prio - MAX_RT_PRIO]; */
    /* p->se.load.inv_weight = prio_to_wmult[p->static_prio - MAX_RT_PRIO]; */

    int prio = p->static_prio - MAX_RT_PRIO;
    struct load_weight *load = &p->se.load;

    /*
     * SCHED_IDLE tasks get minimal weight:
     */
    if (p->policy == SCHED_IDLE) {
        load->weight = scale_load(WEIGHT_IDLEPRIO);
        load->inv_weight = WMULT_IDLEPRIO;                // IDLE进程的权重非常小
        return;
    }

    load->weight = scale_load(prio_to_weight[prio]);
    load->inv_weight = prio_to_wmult[prio];

}
...
static inline void update_load_add(struct load_weight *lw, unsigned long inc)  // enqueue时添加进程se.load.weight到所在rq的load
{
    lw->weight += inc;
    lw->inv_weight = 0;
}
/* static inline void inc_load(struct rq *rq, const struct task_struct *p) */  // deprecated
/* { */
/*     update_load_add(&rq->load, p->se.load.weight); */
/* } */
static void inc_nr_running(struct task_struct *p, struct rq *rq)
{
    rq->nr_running++;

#ifdef CONFIG_NO_HZ_FULL
    if (rq->nr_running == 2) {
        if (tick_nohz_full_cpu(rq->cpu)) {
            /* Order rq->nr_running write against the IPI */
            smp_wmb();
            smp_send_reschedule(rq->cpu);
        }
    }
#endif
}
```

<!--sec8.3-->
### 优先级跟权重的关系图
![scheduler](https://raw.github.com/eslab2013-ustc/RTFSC_Linux_Kernel/master/%E5%8E%86%E6%AC%A1%E7%A0%94%E8%AE%A8%E4%BC%9A%E6%96%87%E6%A1%A3/04-%E8%BF%9B%E7%A8%8B%E7%AE%A1%E7%90%86/%E4%BD%95%E5%8D%93%E9%AA%8B/images/scheduler4.png)

<!--sec9-->
##Thanks~
[presentation link](http://home.ustc.edu.cn/~orcking/reveal.js/scheduler.html)
