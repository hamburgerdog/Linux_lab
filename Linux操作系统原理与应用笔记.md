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
          * pgt_entry = phy_add |PTE_USR|PTE_RW|PTE_PRE;  //  页表项与物理内存真正形成映射
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

## 第四章 内存管理

## 第五章 中断和异常

## 第六章 系统调用

## 第七章 内核中的同步

## 第八章 文件系统

## 第九章 设备驱动




```

```