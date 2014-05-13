<!--Meta fname:mem1 author:'farseer' theme:'moon' title:mem-->
<!--sec1.1-->

## 通用的高速缓存存储结构

![cache](https://raw.github.com/eslab2013-ustc/RTFSC_Linux_Kernel/master/历次研讨会文档/11-内存管理/何卓骋/images/cache.png)

<!--sec1.2-->

* 高速缓存的大小（容量） `C=S*E*B`
* 当一条加载指令指示CPU从主存地址A中读一个字时, 将A发送到高速缓存
    1. 组选择: 检查组索引位，确定组号
    2. 行匹配: 当设置了有效位，并且t个标记位匹配时，匹配到行，否则缓存不命中
    3. 字抽取: 根据b个块偏移位给出字偏移
* 如果缓存不命中，从下层的存储器中取出被请求的块，然后将新的块存储在索引位指示的组中的一个行中。
* 如果组中行满，做合适的替换
    * 最不常使用 LFU
    * 最近最少使用 LRU

<!--sec1.3-->

### 为什么用中间位来做索引？

![cache2](https://raw.github.com/eslab2013-ustc/RTFSC_Linux_Kernel/master/历次研讨会文档/11-内存管理/何卓骋/images/cache2.png)

* 如果用高位作索引，那么一些连续的存储器块，就会映射到相同的高速缓存块
* 一般的程序都有较好的空间局部性，这样容易导致不命中，甚至“抖动”（反复地加载和驱逐相同的高速缓存块的组）

<!--sec1.4-->

### 有关写的问题

1. 如果更新了缓存，何时更新低层存储器中的内容？(写命中)
    * 直写(write-through)
    * 写回(write-back):
        * 能显著减少总线流量
        * 增加了复杂性，需要维护一个额外的修改位(dirty bit)
2. 如果写不命中
    * 写分配(write-allocate), 加载到高速缓存，更新高速缓存
    * 非写分配(not-write-allocate), 避开高速缓存，直接写低一层。

* 直写一般于非写分配对应，写回一般与写分配对应。
* 处理器cr0寄存器的CD标志位用来禁用/启用高速缓存，NW标志指明写策略

<!--sec1.5-->

### 多核高速缓存层次结构

![cache3](https://raw.github.com/eslab2013-ustc/RTFSC_Linux_Kernel/master/历次研讨会文档/11-内存管理/何卓骋/images/cache3.png)

* 一个CPU修改了它的高速缓存，它必须通知其他CPU用适当的值更新(cache snooping), 硬件处理，内核无需关心。
* linux忽略l1, l2, l3的区分，假定只有一个单独的高速缓存

<!--sec2.1-->

### 结合高速缓存和虚拟存储器

访问SRAM是使用虚拟地址还是物理地址？
* 大多数系统是选择物理地址的，方便多进程同时在高速缓存中共享存储块

![cache4](https://raw.github.com/eslab2013-ustc/RTFSC_Linux_Kernel/master/历次研讨会文档/11-内存管理/何卓骋/images/cache4.png)

<!--sec2.2-->

### 利用TLB加速地址翻译

* Translation Lookaside Buffer
* TLB是一个小的，虚拟寻址的缓存，每一行都保存着一个由单个PTE组成的块。
* 高度的相连性
* 地址的翻译步骤由MMU硬件执行，非常快

![TLB](https://raw.github.com/eslab2013-ustc/RTFSC_Linux_Kernel/master/历次研讨会文档/11-内存管理/何卓骋/images/TLB.png)

* T=2^t 个组
* 可以看作通用的缓存格式

<!--sec2.3-->

### 操作视图

![TLB2](https://raw.github.com/eslab2013-ustc/RTFSC_Linux_Kernel/master/历次研讨会文档/11-内存管理/何卓骋/images/TLB2.png)

1. CPU 产生一个虚拟地址
2. MMU 读出虚拟地址中的VPN部分
3. 从TLB中取出相应的PTE
4. MMU将PTE翻译成物理地址
5. 返回字数据

如果TLB不命中，从高速缓存中取出相应的PTE，放到TLB中，可能会覆盖一个已经存在的条目

<!--sec3.1-->

### 多级页表的地址翻译

![paging](https://raw.github.com/eslab2013-ustc/RTFSC_Linux_Kernel/master/历次研讨会文档/11-内存管理/何卓骋/images/paging.png)

<!--sec3.2-->

### 物理地址扩展(PAE) 分页机制

* 虚拟空间还是只有4GB，但是DRAM可以有64GB
* 对应虚拟地址32bit 但是物理地址36bit
* 页目录项中的页大小标志Page size启用大尺寸页（2MB）
* 24bit 2^24个页框， 12bit 标志位
* 页表项大小64bit
* 32bit 虚拟--> 36bit 物理: 4KB 页
    1. cr3 指向PDPT
    2. 31-30 指向PDPT中4项中的一个
    3. 29-21 指向目录中512个项中的一个
    4. 20-12 指向页表中512项中的一个
    5. 11-0  4kb页中的offset
* 32bit 虚拟--> 36bit 物理: 2MB 页
    1. cr3 指向PDPT
    2. 31-30 指向PDPT中4个项之一
    3. 29-21 指向页目录中512个项之一
    4. 20-0  2MB页中的offset
* 通过设置cr4中的PAE标志激活

<!--sec3.3-->

### core i7/linux 存储器系统

![i7](https://raw.github.com/eslab2013-ustc/RTFSC_Linux_Kernel/master/历次研讨会文档/11-内存管理/何卓骋/images/i7.png)

<!--sec3.4-->

![i7_trans](https://raw.github.com/eslab2013-ustc/RTFSC_Linux_Kernel/master/历次研讨会文档/11-内存管理/何卓骋/images/i7_trans.png)

<!--sec3.5-->

![i7_trans2](https://raw.github.com/eslab2013-ustc/RTFSC_Linux_Kernel/master/历次研讨会文档/11-内存管理/何卓骋/images/i7_trans2.png)

* 48bit (256TB) 虚拟地址
* 52bit (4PB) 物理地址
