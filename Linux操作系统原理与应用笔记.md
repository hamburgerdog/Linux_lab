[TOC]

# Linux操作系统原理与应用笔记

## 第一章 概述

### linux内核的技术特点

以实用性和效率为出发点，内核设计成==单内核结构==，整体上把内核作为一个大过程来实现，内核其实就函数和数据结构的集合，其与微内核相比可扩展性和可移植性较低，但与微内核不同，在与文件管理、设备驱动、虚拟内存管理、进程管理等其他上层模块之间不需要有较高的通信开销，==模块之间可以直接调用相关的函数==。（整体的概念）

### linux内核中链表的实现及应用

> ​	双链表通过前趋和后继两个指针域就可以从两个方向循环双链表，如果打乱前趋后继的依赖关系，就可以构成**“二叉树”**、“循环链表”，设计更多的指针域还可以构成各种复杂的树状数据结构，如果减少一个指针域，还可以进一步设计成**“栈”和“队列”**。

* 链表的定义

  ```c
  struct list_head{
    struct list_head *next,*prev;
  }
  
  struct my_list{
    void *mydata;
    struct list_head list;
  }
  ```

  特点：list域隐藏了链表的指针特性，且一个结构中可以有多个list域

* **链表的操作**
  链表头初始化操作为：把前趋后继都指向自己，后续添加操作就是形成一个==循环链表==，内核代码`list.h`中定义了两个宏来定义链表头：

  ```c
  #define LIST_HEAD_INIT(name){&(name),&(name)}
  #define LIST_HEAD(name) struct list_head name = LIST_HEAD_INIT(name)
  ```

  添加节点的具体操作

  ```c
  //	链表的添加节点
  
  /*
  *	静态内联函数 inline说明该函数对编译程序是可见的
  */
  static inline void __list_add(
  	struct list_head *new,
    struct list_head *prev,
    struct list_head *next
  ){
    next->prev=	new;
    new->next	=	next;
    new->prev	=	prev;
    prev->next=	new;
  }
  
  //	在链表头尾添加节点
  static inline void list_add(struct list_head *new,struct list_head *head){
    __list_add(new,head,head->next);
  }
  
  //	在链表头节点后插入new节点
  static inline void list_add_tail(struct list_head *new,struct list_head *head){
    __list_add(new,head->prev,head);
  }
  ```

  `__list_add`这个内部函数，可以看成是==在两个节点(prev节点和next节点)中插入一个新的节点==，这个设计十分巧妙，只要对其进行相应的封装就可以实现多种功能，如`list_add`和`list_add_tail` 这两个函数就可看出，一个是在head节点和后继节点间插入新节点，一个是在head节点和前趋节点间插入，可以用来分别实现**一个栈和一个队列**。

  * *关键字inline必须与函数定义体放在一起才能使函数成为内联，inline函数一般放在头文件中使用*

* **循环链表操作**

  ```c
  //	每次只找出节点在链表中的偏移位置，还需要list_entry来找出节点的起始地址
  #define list_for_each(pos,head)\
  	for(pos=(head)->next; pos!=(head); pos=pos->next)
  
  /*
  *	(char *)(ptr)-(unsigned long)(&((type *)0)->member)
  *	ptr指向的是某一结构list域的绝对地址，type是某一结构，member是type结构中的某一域
  * __返回值__ type *
  */
  #define list_entry(ptr,type,member)\
  	((type *)((char *)(ptr)-(unsigned long)(&((type *)0)->member)))
  
  //	安全的遍历节点，在释放节点前先记录下下一个要释放的节点，因为删除节点后
  //	当前被删除的节点的前趋后继是指向内核中两个固定位置的，如果按list_for_each会出错
  #define list_for_each_safe(pos,n,head)\
  	for(pos=(head)->next,n=pos->next; pos!=(head); pos=n,n=pos->next)

  static inline void __list_del(struct list_head *prev,struct list_head *next){
    next->prev=prev;
    prev->next=next;
  }
  static inline void list_del(struct list_head *entry){
    __list_del(entry->prev,entry->next);
    entry->next=LIST_POSTION1;	//	内核地址中的固定地址
    entry->prev=LIST_POSTION2;
  }
      
  //	具体代码中的应用
  struct numlist{
    int num;
    struct list_head list;
  }
  ...
    struct numlist numhead;
  	INIT_LIST_HEAD(&numhead.list);
  
    struct numlist *listnode;
  	struct list_head *pos;
  	struct numlist *p;
  	//	遍历节点的操作
  	list_for_each(pos,&numhead.list){
      // 	list(prt,type,member)
      p = list_entry(pos,struct numlist,list);
      printk(... p->num);
    }
  ...
    - - - - - - - - - - - - - - - - - - - - - 
  ...
    struct list_head *n;
  	//	删除所有节点的操作
  	list_for_each_safe(pos,n,&numhead.lits){
      list_del(pos);
      p = list_entry(pos,struct numlist,list);
      kfree(p);
    	...
    }
  ...
  ```
  
  具体分析`list_entry`:把0地址转换成type类型的指针，获取该结构中member域的指针，也就是为了==得到member在type结构中的偏移量==，而==ptr - member==就得到了type结构的起始地址，也就**获得某一节点的起始地址**，可以进一步读出type结构中的data域和list域
  
  哈希表也是链表的一种衍生，在`list.h`中也有相关实现

## 第二章 内存寻找
### 内存寻址

> 保护模式：这种模式下内存段的范围受到了限制，范围内存时不能直接从段寄存器中获得段的起始地址，而需要经过额外转换和检查（从此不能再随意存取数据段）
>
> 线性地址：指一段连续的、不分段的，范围从0~4GB的地址空间，一个线性地址就是线性空间的一个绝对地址

80386中地址总线和数据总线都是32位，寻址能力达到了4GB，但其为了兼容，还是保留了16位的段寄存器，并采用了在段寄存器基础上的方式来构筑保护机制，即寻址方式为：==段选择符：偏移地址（虚拟地址）==->线性地址（使用MMU转换）->物理地址，**段寄存器中存放的是段选择符**（简单理解为段描述表的索引）。

16位的段寄存器是明显不足以确定一个基地址，因此段寄存器里存放的段选择符就要发挥作用了，同时在保护模式下，系统中存放有三种类型的描述符表：GDT、IDT（中断描述符表）、LDT，为了加快读取速度还设计了三个寄存器，通过段选择符加描述符表的地址，就可以取得段描述符。

Linux为了保持可移植性并不真正地使用分段机制，开发人员巧妙地将所有段的基地址设置成0，因此所有的进程都共享了0~4GB的线性空间，这样“偏移量”就等于了“线性地址”，也就是说**虚拟地址就直接等同于了线性地址**，但这样会让段保护的第一个方法无法发挥作用，且如果线性空间直接映射到物理空间，还会出现进程使用的地址互相覆盖的问题，为此Linux使用了分页机制来解决问题。

页对应的是物理内存的块，大小都是4KB，通常采用两级页表（页目录和页表）的方法来实现线性地址到物理地址的映射，**32位线性地址**转换成**物理地址**的处理方式为：

1. 最高10位为页目录项的索引，其左移两位后与CR3中的页目录基地址相加可以得到对应的页目录项地址
2. 中间10位为页表项的索引，其左移两位后与从页目录项得到的页表基址相加得到具体的页表项
3. 最低12位为页面偏移地址，从页表项中映射到页面的物理基址，与偏移地址相加就可得到要找的物理地址

### Linux具体实现内存寻址的方式  

目前很多平台都开始使用64位的处理器，Linux为了兼容使用了三级页表的机制，但当前讨论还是通过二级页表的模式为主，其三级页表具体设计为：线性地址（总目录：中间目录：页表：偏移量）。仅支持二级页表的处理器上使用三级页表的模式时，Linux把中间目录当成只有一项，并把其“折叠”到总目录之中，从而适应了二级页表的机制。

Linux中每一个进程都有自己的页目录和页表集，当进程发生切换时，**Linux把CR3的内容存放到前一个执行进程的PCB中**，而把下一个要执行的进程的PCB的值装入到CR3中，恢复进程的时候，Linux会先查找PCB中的暂存的内容并恢复到CR3中，从而使分页单元指向正确的页表

**linux内核初始化页表的代码实现**

  ```c
  #define NR_PGT 0x4   												//  需要初始化的页面个数
  #define PGD_BASE (unsigned int *)0x1000     //  页目录表映射到物理内存的地址
  #define PAGE_OFFSET (unsigned int)0x2000    //	页表的起始地址	

  #define PTE_PRE 0x01    // 初始化时 页表会装入内存
  #define PTE_RW  0x02    // 与U/S位形成硬件保护 
  #define PTE_USR 0x04    // Page Write-Through 写透方式

  void page_init(){
      int pages = NR_PGT; 										//  系统初始化时创建4个页表
      unsigned int page_offset = PAGE_OFFSET;
      unsigned int * pgd = PGD_BASE;          //  页目录表存放的物理地址
      while (pages--)
      {
          * pgd++ = page_offset |PTE_USR|PTE_RW|PTE_PRE;  //  创建四个页目录表项
          page_offset += 0x1000   												//  每个页目录表的大小为2^12=4KB
      }
      pgd = PGD_BASE;

      //  页表从物理内存第三个页框开始
      unsigned int *pgt_entry = (unsigned int *)0x2000;   
      unsigned int phy_add = 0x0000;
      //  0x1000000=16MB 初始化了四个页表，每个页表映射了4MB的物理内存地址
      while (phy_add < 0x1000000) 
      {
          * pgt_entry = phy_add |PTE_USR|PTE_RW|PTE_PRE;  //  页面与物理内存真正形成映射
          phy_add += 0x1000;      												//  物理块大小和页面大小都是4KB
      }

      //  CR0最高位为控制分页位，linux下分页机制的开启是可选的，则段内嵌汇编的作用就是允许分页
      __asm__ __volatile__("movl  %0, %%cr3;"
                                  "movl   %%cr0, %%eax;"
                                  "orl    $0x80000000, %%eax;"
                                  "movl   %%eax, %%cr0;"::"r"(pgd):"memory","%eax");

  }
  ```



## 第三章 进程

### linux系统中的进程控制块

linux中对进程的描述结构叫做PCB（task_struct）其是一个相当庞大的结构体，按功能可以分成以下几类

1. 状态信息-描述进程的动态变化
2. 链接信息-描述进的亲属关系
3. 各种标识符
4. 进程间通信信息
5. 时间和定时器信息
6. 调度信息
7. 文件系统信息
8. 虚拟内存信息-描述进程编译连接后形成的地址空间
9. 处理器环境信息-进程的执行环境（处理器的各种寄存器及堆栈信息），==体现进程动态变化最主要的场景==

系统创建一个新进程的时候就是在内核中为它建立了一个PCB，进程结束的时候又收回PCB，其是内核中频繁读写的数据结构，因此应当常驻内存。

每当进程从用户态进入内核态后都要使用栈-进程的内核栈，进程一进入内核态，CPU就自动为其设置该进程的内核栈，这个栈位于**内核的数据段**上，其==*内核栈和一个`thread_info`结构存放在一起，大小为8KB*==。实际上内核为PCB分配空间的方式是动态的（**确切地说，内核根本不为PCB分配内存**），而仅仅给内核栈分配8KB的内存，并把一部分让给PCB使用(thread_info)。

段起始于末端，并朝这个内存区开始的方向增长，从用户态转到内核态以后，<u>进程的内核栈总是空的</u>，堆栈寄存器ESP直接指向内存区的顶端，只要把数据写入栈中，ESP的值递减。`thread_info`与内核栈存放在一起的最大好处是，内存栈很容易从`ESP`的值获取到当前CPU上运行的`thread_info`结构的地址，因为`thread_union`(内核栈和thread_info)结构的长度是8KB，**则内核屏蔽ESP的低13位就得到thread_info结构的基地址**，通过`*task`就可以得到该进程的PCB，`PCB`和`thread_info`都有一个域是指向对方的，是一种一一对应的关系，而再定义一个`thread_info`结构的原因有两种可能：1.该结构是最频繁被调用的 2.随着linux版本的变化，PCB越来越大，为了节省内核栈的空间，需要把一部分的PCB内容移出内核栈，只保留最频繁被使用的`thread_info` 

### linux中进程的组织方式

内核建立了几个进程链表，双向循环链表的头尾都是`init_task`（0号进程的PCB，是预先由编译器静态分配到内核数据段的，在运行过程中保持不变，永远不会被撤销的），系统使用哈希表和链地址法来加速用PID找到相应PCB的过程，并组织好了一个就绪队列和等待队列

* 就绪队列存放处于就绪态和运行态的进程

* 等待队列存放睡眠进程，对中断处理、进程同步和定时用处很大

  ```c
  //	等待队列的数据结构
  struct __wait_queue{
    unsigned init flages;	//	区分互斥进程和非互斥进程，对于互斥进程值为（WQ_FLAG_EXCLUSIVE）
    #define WQ_FLAG_EXCLUSIVE 	0x01	
    void * private;							//	传递给func的参数
    wait_queue_func_t func;			//	用于唤醒进程的函数，需要根据等待的原因归类
    struct list_head task_list;	//	用于组成等待队列
  };
  typedef struct __wait_queue wait_queue_t;
  
  //	等待队列头结构
  /*
  *	等待队列是由中断处理程序和主要内核函数修改的,因此必须对其双向链表保护,以免对其进行同时访问
  *	所以采用了自旋锁来进行同步
  */
  struct __wait_queue_head{
    spinlock_t lock;
    struct list_head task_list;
  }
  ```

  等待队列是由中断处理程序和主要内核函数修改的，因此必须对其双向链表保护，以免对其进行同时访问，所以采用了自旋锁来进行同步

  等待队列的操作`add_wait_queue()`把一个非互斥进程插入到等待队列链表的第一个位置，`add_wait_queue_exclusive()`把一个互斥进程插入但等待队列的最后一个位置。让某一个进程去睡眠的最基本操作为：先把当前进程的状态设置成`TASK_UNINTERRUPTIBLE`并把它插入到特定的等待队列中，然后调用调度程序，当进程被唤醒的时候会接着执行剩余的指令，同时把进程从等待队列中删除

  ```c
  //	wake_up()函数
  void wake_up(wait_queue_head_t){
    struct list_head *tmp;
    wait_queue_t *curr;
    //	扫描链表，找等待队列中的所有进程
    list_for_each(tmp,&q->task_list){
      //	curr指向每个等待进程的起始地址
      curr=list_entry(tmp,wait_queue_t,task_list);
      /*如果进程已经被唤醒并且进程是互斥的，则循环结束
       *因为所有的非互斥进程都是在链表的开始位置，而所有的互斥进程都在链表的尾部，所以可以先唤醒非互斥			 *进程再唤醒互斥进程
       */
      if(curr->func(curr,TASK_INTERRUPTIBLE|TASK_UNINTERRUPTIBLE,0,NULL)&&curr->flags)
        break;
    }
  }
  ```

### linux进程调度

linux进程调度是时机：

	1. 进程状态转换的时刻，使用`sleep_on()`、`exit()`时会主动调用调度函数
 	2. 当前进程的时间片用完
 	3. 设备驱动程序运行时
 	4. 从内核态返回到用户态时，从系统调用返回意味着离开内核态，状态转换需要花费一定的时间，在返回到用户态前，系统把在内核态该处理的事应当全部做完。

```c
//	schedule() 函数主框架 
static void __sched notrace __schedule(bool preempt)
{
    struct task_struct *prev, *next;
    unsigned long *switch_count;
    struct rq *rq;
    int cpu;

    /*  ==1==  
        找到当前cpu上的就绪队列rq
        并将正在运行的进程curr保存到prev中  */
    cpu = smp_processor_id();
    rq = cpu_rq(cpu);
    prev = rq->curr;

    /*
     * do_exit() calls schedule() with preemption disabled as an exception;
     * however we must fix that up, otherwise the next task will see an
     * inconsistent (higher) preempt count.
     *
     * It also avoids the below schedule_debug() test from complaining
     * about this.
     */
    if (unlikely(prev->state == TASK_DEAD))
        preempt_enable_no_resched_notrace();

    /*  如果禁止内核抢占，而又调用了cond_resched就会出错
     *  这里就是用来捕获该错误的  */
    schedule_debug(prev);

    if (sched_feat(HRTICK))
        hrtick_clear(rq);

    /*  关闭本地中断  */
    local_irq_disable();

    /*  更新全局状态，
     *  标识当前CPU发生上下文的切换  */
    rcu_note_context_switch();

    /*
     * Make sure that signal_pending_state()->signal_pending() below
     * can't be reordered with __set_current_state(TASK_INTERRUPTIBLE)
     * done by the caller to avoid the race with signal_wake_up().
     */
    smp_mb__before_spinlock();
    /*  锁住该队列  */
    raw_spin_lock(&rq->lock);
    lockdep_pin_lock(&rq->lock);

    rq->clock_skip_update <<= 1; /* promote REQ to ACT */

    /*  切换次数记录, 默认认为非主动调度计数(抢占)  */
    switch_count = &prev->nivcsw;

    /*
     *  scheduler检查prev的状态state和内核抢占表示
     *  如果prev是不可运行的, 并且在内核态没有被抢占
     *  
     *  此时当前进程不是处于运行态, 并且不是被抢占
     *  此时不能只检查抢占计数
     *  因为可能某个进程(如网卡轮询)直接调用了schedule
     *  如果不判断prev->stat就可能误认为task进程为RUNNING状态
     *  到达这里，有两种可能，一种是主动schedule, 另外一种是被抢占
     *  被抢占有两种情况, 一种是时间片到点, 一种是时间片没到点
     *  时间片到点后, 主要是置当前进程的need_resched标志
     *  接下来在时钟中断结束后, 会preempt_schedule_irq抢占调度
     *  
     *  那么我们正常应该做的是应该将进程prev从就绪队列rq中删除, 
     *  但是如果当前进程prev有非阻塞等待信号, 
     *  并且它的状态是TASK_INTERRUPTIBLE
     *  我们就不应该从就绪队列总删除它 
     *  而是配置其状态为TASK_RUNNING, 并且把他留在rq中

    /*  如果内核态没有被抢占, 并且内核抢占有效
        即是否同时满足以下条件：
        1  该进程处于停止状态
        2  该进程没有在内核态被抢占 */
    if (!preempt && prev->state)
    {

        /*  如果当前进程有非阻塞等待信号，并且它的状态是TASK_INTERRUPTIBLE  */
        if (unlikely(signal_pending_state(prev->state, prev)))
        {
            /*  将当前进程的状态设为：TASK_RUNNING  */
            prev->state = TASK_RUNNING;
        }
        else   /*  否则需要将prev进程从就绪队列中删除*/
        {
            /*  将当前进程从runqueue(运行队列)中删除  */
            deactivate_task(rq, prev, DEQUEUE_SLEEP);

            /*  标识当前进程不在runqueue中  */
            prev->on_rq = 0;

            /*
             * If a worker went to sleep, notify and ask workqueue
             * whether it wants to wake up a task to maintain
             * concurrency.
             */
            if (prev->flags & PF_WQ_WORKER) {
                struct task_struct *to_wakeup;

                to_wakeup = wq_worker_sleeping(prev);
                if (to_wakeup)
                    try_to_wake_up_local(to_wakeup);
            }
        }
        /*  如果不是被抢占的，就累加主动切换次数  */
        switch_count = &prev->nvcsw;
    }

    /*  如果prev进程仍然在就绪队列上没有被删除  */
    if (task_on_rq_queued(prev))
        update_rq_clock(rq);  /*  跟新就绪队列的时钟  */

    /*  挑选一个优先级最高的任务将其排进队列  */
    next = pick_next_task(rq, prev);
    /*  清除pre的TIF_NEED_RESCHED标志  */
    clear_tsk_need_resched(prev);
    /*  清楚内核抢占标识  */
    clear_preempt_need_resched();

    rq->clock_skip_update = 0;

    /*  如果prev和next非同一个进程  */
    if (likely(prev != next))
    {
        rq->nr_switches++;  /*  队列切换次数更新  */
        rq->curr = next;    /*  将next标记为队列的curr进程  */
        ++*switch_count;    /* 进程切换次数更新  */

        trace_sched_switch(preempt, prev, next);
        /*  进程之间上下文切换    */
        rq = context_switch(rq, prev, next); /* unlocks the rq */
    }
    else    /*  如果prev和next为同一进程，则不进行进程切换  */
    {
        lockdep_unpin_lock(&rq->lock);
        raw_spin_unlock_irq(&rq->lock);
    }

    balance_callback(rq);
}
STACK_FRAME_NON_STANDARD(__schedule); /* switch_to() */

/*转载自： http://blog.csdn.net/gatieme*/

/* 进程地址空间切换详解 */
kstat.context_swtch++;	//	统计上下文切换的次数
{
  struct mm_struct *mm = next -> mm;
  struct mm_struct *oldmm = prev -> active_mm;
  if(!mm){		//	没有用户空间，表明这为内核线程
    if(next->active_mm==NULL)BUG();
    nexit->active_mm=oldmm;
  }else{			//	一般进程则切换到这段用户空间
    if(next->active_mm!=mm)BUG();
    switch_mm(oldmm,mm,next,this_cpu);
  }
  if(!prev->mm){		//	切换出去的是内核线程的处理方式
    prev->active_mm=NULL;
    mmdrop(oldmm);
  }
}

```

Linux schedule()分析：

1. 进程需要有自己的地址空间，或者和其他进程借用，如果都没有则出错，且如果`schedule()`在中断服务程序内部执行也出错
2. 对当前进程要做相关的处理，应当进入调度程序是，其状态不一定还是`TASK_RUNNNING`
3. 进程地址空间切换，如果新进程有自己的用户空间，则`switch_mm()`函数会把该进程从内核空间转换到用户空间（加载下一个要执行的进程的页目录）；如果新进程是一个内核线程，无用户空间而在内核空间中运行，则要借用前一个进程的地址空间，因为所有的进程的内核空间都是共享的。如果切换出去的如果是内核线程，则要归还所借用的地址空间，并把mm_struct 中的共享计数减1

### Linux进程创建、线程及其创建

Linux创建进程的方式是通过`fork()`或者`clone()`，然后再调用`exec()`，其使用的是写时复制技术（把父子进程的全部资源都设为只读，在父子进程尝试对其进行修改时才将被修改前的全部资源复制给子进程），创建进程的实际花销是为其创建PCB并把父进程的页表拷贝一份，如果进程中包含线程，则所有线程共享这些资源，无须拷贝。子进程一开始处于深度睡眠态，以确保它不会立刻运行，在把进程PCB插入到进程链表和哈希表后才将其设成就绪态，并让其平分父进程剩余的时间片，内核有意让子进程先执行，是为了让子进程使用`exec()`去执行其自己的代码，避免父进程操作引起写时复制，提高系统运行速度

Linux把线程看成一个使用某些共享资源的进程，每个线程有唯一的PCB，一般情况下内核线程会在创建时永远地执行下去，在需要的时候就会被唤醒和执行。

1. 进程0：内核初始化工作的`start_kernel()`创建一个内核线程也就是进程0，其PCB就是`init_task`
2. 进程1：也就是init进程，其一开始是一个内核线程，其调用了`execve()`装入了用户态下可执行程序init(/sbin/init)，因此init是内核线程启动起来的一个普通进程，也就是用户态下的第一个进程

## 第四章 内存管理

32位平台线性空间固定大小为4GB，其中高地址1GB（0xC000 0000~0xffff ffff）是内核空间，被内核使用并且由所有进程共享，每个用户进程的用户空间为3GB大小，通过分页机制实现各个进程的用户空间私有。

进程页目录PGB就位于内核空间中，在切换进程的时候需要将CR3指向新进程的PGB，CR3需要物理地址，而PGB在内核中的起始地址是虚地址，这时候需要转换，Linux的内核空间有一个独特设计，即==内核空间连续地占据了每个虚拟空间中最高的1GB，映射到物理内存却总是从最低地址开始的==，因此内核地址到物理地址只需要减去`PAGE_OFFSET`就可以了。

内核地址空间的结构：内核的代码和数据叫做内核映像，Linux内核映像存放于0x0010 0000开始的地方

1. 这前1M的空间用于存放于系统硬件相关的代码和数据
2. 内核映像占用0x10 0000到start_mem的空间
3. Start_mem到end_mem这段区域叫做动态内存，是用户程序和数据使用的内存区

### 进程的用户空间管理

用户地址空间的结构：用户程序经过编译和链接后形成二进制映像文件，数据段、代码段、堆栈使用的空间都是在建立进程的时候就分配好，都属于必需的基本要求

1. 堆栈段：在用户空间顶部，由顶向下延伸
2. BSS：动态分配的空间
3. 数据段：静态分配的数据空间，
4. 代码段：程序的相关代码

每个进程只有一个`mm_struct`，其是对整个用户空间的描述，而一个进程的虚拟空间中可能有多个虚拟区间，用`vm_area_struct`描述，如堆栈段、数据段......

* `mm_struct`	在 `task_struct` 可以找到指向该结构的指针，虽然每个进程只有一个虚拟地址空间，但是该空间可以被其他进程所共享，因此需要使用原子类型的操作 `atomic_t`(该结构中包含了一个计数器)，**描述了代码段、数据段、参数段已经环境段的起始地址和结束地址**，==还有指针pgt指向该进程的页目录==

  > ***进程页表和内核页表的区别*** - [Linux 内核页表和进程页表](https://blog.csdn.net/chuba6693/article/details/100612637)
  >
  > * 在保护模式下，**从硬件角度看，其运行的基本对象为“进程”(或线程)，而寻址则依赖于“进程页表”**，在进程调度而进行上下文切换时，会进行页表的切换：即将新进程的pgd(页目录)加载到CR3寄存器中。
  > * **进程页表中的线性地址包括两个部分：用户态和内核态**，内核态地址对应的相关页表项，对于所有进程来说都是相同的(因为**内核空间对所有进程来说都是共享的**)，而这部分页表内容其实就来源于“内核页表”，即每个进程的“进程页表”中内核态地址相关的页表项都是“内核页表”的一个拷贝。
  > * **内核页表也包括两个部分：线性映射区和vmalloc区**，“内核页表”由内核自己维护并更新，在`vmalloc区`发生`page fault`时，将“内核页表”同步到“进程页表”中。
  > * 以`vmalloc`为例(最常使用)，这部分区域对应的线性地址在内核使用`vmalloc`分配内存时，其实就已经分配了相应的物理内存，并做了相应的映射，建立了相应的页表项，但**相关页表项仅写入了“内核页表”，并没有实时更新到“进程页表中”，内核在这里使用了“延迟更新”的策略**，将“进程页表”真正更新推迟到第一次访问相关线性地址，发生`page fault`时，此时在`page fault`的处理流程中进行“进程页表”的更新。

* `vm_area_struct` Linux把虚存区看成是对象，把用户空间划分成一段一段是因为每个虚存区的来源可能不同，有的来自可执行映像，有的来自共享库、动态分配的内存区，不同的区有不同的操作权限和操作方法；`vm_area_struct` 可用双向链表和红黑树来组织，有利于快速定位虚存区

创建进程的时候，进程用户空间的创建依赖于父进程，所做的工作仅仅是`mm_struct`和`vm_area_struct`的创建以及页目录和页表的建立，采用**写时复制**的方法。Linux并不把进程的可执行映像装入物理内存，只是把它们链接到进程的用户空间，被引用的程序部分会由操作系统装入物理内存，也就是需要使用请页机制

### 请页机制

给进程分配新物理页面的确定方式：

1. 如果页面不在内存中，页没有被调入，则内核分配一个新页面并初始化，“请求调页”
2. 如果页面在内存但是只读，则内核分配一个新页面并复制旧页面的内容，“写时复制”

* *请求调页：写处理，获取新页面，把页面填为0，把页表置为新页面的物理地址，并设页面为可写和脏；读处理，分配一个零页，零页在内核初始化期间被静态分配并标记为不可写，当进程写该页面的时候才使用写时复制*

### 物理内存

内核用`struct page`结构表示系统中的每一个物理页面，也叫页描述符，这种结构目的在于描述物理内存本身，内核仅用这个数据结构来描述当前时刻在相关物理页中存放的东西。

**伙伴算法**：Linux把空闲页面分为10块链表，每个链表中的一个块为2的幂次个页面，

```c
	struct free_area_struct{
    struct page *next;		//	用于将page链接成一个双向链表
    struct page *prev;		
    unsigned int *map;		//	map指向一个位图
  }free_area[10];
```

算法过程：如果要求分配的块大小为128个页面，则去块大小为128的链表中找，如果没有则往上找，如果256大小的链表中有空间，则把256个页面平分，高地址的被使用，低地址的加入128的链表中，回收过程则相反，同时要注意相邻的物理页面要进行合并

Linux中有`freepages`结构，来使用内核交换守护进程(`kswapd`)保证系统有足够的物理内存，结构中有`min|low|high`三条线，各个界限值是通过实际的物理内存大小计算出来的，少于low会开启强交换；少于high会启动后台交换；高于high则什么都不做。

**Slab分配机制**：用于解决内碎片，减少对伙伴算法的调用次数。对于预期频繁使用的内存区可以创建特定大小的专业缓冲区来处理，使用较少的内存区创建通用缓冲区来处理。

* slab缓冲区由一连串的大块slab构成，每个大块中包含了若干个同类型的对象，实际上缓冲区是内存中的一片区域，这片区域划分为多个slab块，每个slab由一个或者多个页面组成，存放的都是同一类型的对象 
* 通用缓冲区，通用缓冲区最小的为32B、64B.....128KB，对通用缓冲区的管理依旧是slab方式

> `kmalloc()`用于分配内核中的连续内存   |   `vmalloc()`用于分配非连续的内核内存

### 回收机制

把页面换出推迟到无法推迟为止，换出页面的时候不需要先将内容写入到磁盘中，如果一个页面从最近一次换入后并没有被写过则它是干净的，可以一直缓冲到必要时才加以回收；写过的脏页面放到磁盘交换区中，但不立即释放，一直推迟到必要时才进行，如果一个页面在释放后又被访问，则重新从磁盘缓冲区读入即可

内核守护线程`kswapd`是有自己的PCB，一样受到内核的调度，由内核设计时规定多久运行一次，

## 第五章 中断和异常

## 第六章 系统调用

## 第七章 内核中的同步

## 第八章 文件系统

## 第九章 设备驱动




```

```