## 每CPU页框高速缓存

* 每个CPU高速缓存包含一些预先分配的页框， 用于满足本地CPU发出的单一内存请求。(提升系统性能)
* 两个高速缓存: 热/冷
    * 热高速缓存中存放的页框中包含的内容 *很可能* 在CPU硬件的高速缓存中。
    * 从冷高速缓存获得页框为其他类型的内存分配保存了热页框储备
* 如果在刚分配到页框后就立即向页框写，那么用热高速缓存对性能有利。
    * 每次对页框存储单元的访问将都会导致原来一个存在于硬件高速缓存的一页被替换掉 (不命中)
    * 除非硬件高速缓存包含有一行：它映射刚被访问的 “热”页框单元 (命中)
* 如果将要被DMA操作填充，那么用冷高速缓存是方便的。
    * 不涉及到CPU
    * 硬件高速缓存的行不会被替换。

### 数据结构

* pageset数组 per_cpu_pageset 类型。
* 每个pageset的成员有两个per_cpu_pages, 一个留给冷，一个热。
* per_cpu_pages字段:

| 类型             | 名称  | 描述                                   |
|------------------|-------|----------------------------------------|
| int              | count | 高速缓存中的页框个数                   |
| int              | low   | 下界，表示高速缓存需要补充             |
| int              | high  | 上界，表示高速缓存用尽                 |
| int              | batch | 在高速缓存中将要添加或被删去的页框个数 |
| struct list_head | list  | 高速缓存中包含的页框描述符链表         |

* 改进为lists 数组，MIGRATE_PCPTYPES个链表，每个针对一种 migrate type
* 如果页框个数count低于下界low，内核通过伙伴系统中分配batch个单一页框来补充对应的高速缓存。
* 如果页框个数高过上届high，内核从高速缓存中释放batch个页框到伙伴系统中。

![pcp](https://raw.github.com/eslab2013-ustc/RTFSC_Linux_Kernel/master/%E5%8E%86%E6%AC%A1%E7%A0%94%E8%AE%A8%E4%BC%9A%E6%96%87%E6%A1%A3/13-%E5%86%85%E5%AD%98%E7%AE%A1%E7%90%86/%E4%BD%95%E5%8D%93%E9%AA%8B/pcp.png)

``` c
struct per_cpu_pageset {
    struct per_cpu_pages pcp;
#ifdef CONFIG_NUMA
    s8 expire;
#endif
#ifdef CONFIG_SMP
    s8 stat_threshold;
    s8 vm_stat_diff[NR_VM_ZONE_STAT_ITEMS];
#endif
};

struct per_cpu_pages {
    int count;      /* number of pages in the list */
    int high;       /* high watermark, emptying needed */
    int batch;      /* chunk size for buddy add/remove */

    /* Lists of pages, one per migrate type stored on the pcp-lists */
    struct list_head lists[MIGRATE_PCPTYPES];
};
```

### 通过每CPU页框高速缓存分配页框

* buffered_rmqueue() 函数在指定的内存管理区中分配页框。使用每CPU页框高速缓存来处理单一页框请求。参数：
    * 内存管理区描述符的地址zone
    * 请求分配的大小的对数order
    * 分配标志gfp_flags

``` c
struct page *buffered_rmqueue(struct zone *preferred_zone,
            struct zone *zone, int order, gfp_t gfp_flags,
            int migratetype)
{
    unsigned long flags;
    struct page *page;
    int cold = !!(gfp_flags & __GFP_COLD);

again:
    if (likely(order == 0)) {                                                         // 1
        struct per_cpu_pages *pcp;
        struct list_head *list;

        local_irq_save(flags);
        pcp = &this_cpu_ptr(zone->pageset)->pcp;
        list = &pcp->lists[migratetype];
        if (list_empty(list)) {                                                       // 2 low = 0
            pcp->count += rmqueue_bulk(zone, 0, pcp->batch, list, migratetype, cold); // 2.2 2.3
            if (unlikely(list_empty(list)))
                goto failed;
        }

        if (cold)                                                                     // 3
            page = list_entry(list->prev, struct page, lru);
        else
            page = list_entry(list->next, struct page, lru);

        list_del(&page->lru);
        pcp->count--;
    } else {                                                                          // 4
        if (unlikely(gfp_flags & __GFP_NOFAIL)) {
            /*
             * __GFP_NOFAIL is not to be used in new code.
             *
             * All __GFP_NOFAIL callers should be fixed so that they
             * properly detect and handle allocation failures.
             *
             * We most definitely don't want callers attempting to
             * allocate greater than order-1 page units with
             * __GFP_NOFAIL.
             */
            WARN_ON_ONCE(order > 1);
        }
        spin_lock_irqsave(&zone->lock, flags);
        page = __rmqueue(zone, order, migratetype);
        spin_unlock(&zone->lock);
        if (!page)
            goto failed;
        __mod_zone_freepage_state(zone, -(1 << order),
                      get_pageblock_migratetype(page));
    }

    __count_zone_vm_events(PGALLOC, zone, 1 << order);                                // 5
    zone_statistics(preferred_zone, zone, gfp_flags);                                 // preferred_zone for NUMA statistics
    local_irq_restore(flags);

    VM_BUG_ON(bad_range(zone, page));
    if (prep_new_page(page, order, gfp_flags))
        goto again;
    return page;                                                                      // 6

failed:
    local_irq_restore(flags);
    return NULL;
}
```

1. 如果order不等于0，不能使用pcp，直接跳到4
2. 检查`__GFP_COLD` 标志所标识的内存管理区本地每CPU高速缓存是否需要补充(count <= low), 如果是
    1. 通过反复调用`__rmqueue()`函数从伙伴系统中分配batch个单一页框
    2. 将已分配页框的描述符插入高速缓存链表中
    3. 增加count
3. 如果count为正，则函数从高速缓存链表获得一个页框，count--， 跳到第5步。
4. 如果到这里还没有被满足，原因: 请求跨越了几个连续的页框，或者被选中的内存高速缓存为空。调用`__rmqueue()`函数从伙伴系统中分配所请求的页框。
5. 如果内存请求得到满足，函数初始化(第一个)页框的页描述符，清除一些标志。
6. 返回5中得到的页描述符，失败则NULL

* 越靠近链表头的页越热，越靠近链表尾的页越冷，因为每次释放单个页框的时候，页框是插入到链表的头部的，也就是说靠近头部的页框是最近才释放的，因此最有可能存在于高速缓存当中。

### 释放页框到每CPU页框高速缓存

* 为了释放单个页框到每CPU页框高速缓存，使用`free_hot_page()` 和 `free_cold_page()` (没找到这两个函数)
* 都是`free_hot_cold_page()`的简单封装

``` c
void free_hot_cold_page(struct page *page, int cold)
{
    struct zone *zone = page_zone(page);                                   // 1
    struct per_cpu_pages *pcp;                                             // 2
    unsigned long flags;
    int migratetype;

    if (!free_pages_prepare(page, 0))
        return;

    migratetype = get_pageblock_migratetype(page);
    set_freepage_migratetype(page, migratetype);
    local_irq_save(flags);
    __count_vm_event(PGFREE);

    /*
     * We only track unmovable, reclaimable and movable on pcp lists.
     * Free ISOLATE pages back to the allocator because they are being
     * offlined but treat RESERVE as movable pages so we can get those
     * areas back if necessary. Otherwise, we may have to free
     * excessively into the page allocator
     */
    if (migratetype >= MIGRATE_PCPTYPES) {
        if (unlikely(is_migrate_isolate(migratetype))) {
            free_one_page(zone, page, 0, migratetype);
            goto out;
        }
        migratetype = MIGRATE_MOVABLE;
    }

    pcp = &this_cpu_ptr(zone->pageset)->pcp;
    if (cold)
        list_add_tail(&page->lru, &pcp->lists[migratetype]);               // 4
    else
        list_add(&page->lru, &pcp->lists[migratetype]);
    pcp->count++;
    if (pcp->count >= pcp->high) {                                         // 3
        unsigned long batch = ACCESS_ONCE(pcp->batch);
        free_pcppages_bulk(zone, batch, pcp);
        pcp->count -= batch;
    }

out:
    local_irq_restore(flags);
}
```

1. 从page->flags 字段被获取包含该页框的内存管理区描述符地址
2. 获取由cold标志选择的管理区高速缓存的per_cpu_pags描述符地址
3. 检查高速缓存是否应该被清空（count >= high）, 如果是，`free_pages_bulk()`
4. 把释放的页框添加到高速缓存链表上，增加count字段。

* 2.6版本中，从没有页框被释放到冷高速缓存中。
* 对于硬件高速缓存，内核总是假设被释放的页框是热的。

### migrate type

* 2.6版本的后期引入，为了解决碎片问题
* buddy的碎片防止机制 寄托于使用者会及时释放内存。如果使用者长期不释放，或者还未释放，碎片就会存在
* 按照可移动性分类

```c
struct free_area {
    struct list_head    free_list[MIGRATE_TYPES];
    unsigned long        nr_free;
};
```

* 每个type各自占一块足够大的连续物理空间，尽量在自己type所属的空间内完成内存分配。
* 如果某个type的链表中没有可以分配的内存了，那么还是可以从其它链表里暂时抢一些。
* 内核一开始将所有页框都归到*可移动*组，别的都是空，只有真的不可移动需求出现时，再分配一部分作为不可移动的链表
