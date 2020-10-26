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

## 第三章 进程

## 第四章 内存管理

## 第五章 中断和异常

## 第六章 系统调用

## 第七章 内核中的同步

## 第八章 文件系统

## 第九章 设备驱动



