#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/init.h>

unsigned long **sys_call_t1=(unsigned long * *)0xffffffff916013c0; 
unsigned long *orig_saved;	// 用于复原数据

unsigned long clear_and_return_cr0(void){
	unsigned long cr0 = 0;
	unsigned long ret;
	asm("movq %%cr0, %%rax":"=a"(cr0));
	ret = cr0;
	cr0 &= 0xfffffffffffeffff;
	asm ("movq %%rax, %%cr0"::"a"(cr0));
	return ret;
}
//读取val的值到rax寄存器，再将rax寄存器的值放入cr0中
void setback_cr0(unsigned long val){
 	asm volatile("movq %%rax, %%cr0"::"a"(val));
}

int make_rw(unsigned long address){  
	unsigned int level;  
	pte_t *pte = lookup_address(address, &level);//查找虚拟地址所在的页表地址  
	if (pte->pte & ~_PAGE_RW)  //设置页表读写属性
			pte->pte |=  _PAGE_RW;  
		
	return 0;  
}  
  
int make_ro(unsigned long address){  
	unsigned int level;  
	pte_t *pte = lookup_address(address, &level);  
	pte->pte &= ~_PAGE_RW;  //设置只读属性

	return 0;  
}

asmlinkage long sys_mycall(void){
    printk(KERN_ALERT "using my own syscall!\n");
    return 1024;
}

static int __init syscall_init_module(void){    
	unsigned long val;
	printk("sys33_table: %lx", (long )sys_call_t1);//获取系统调用表的地址
	orig_saved = (unsigned long *)(sys_call_t1[335]);  //保存原有的335号的系统调用表的地址
		
	printk("orig_saved : %lx",(long) orig_saved );  

	make_rw((unsigned long)sys_call_t1);  //修改页的写属性
	

		val=clear_and_return_cr0();
	sys_call_t1[335] = (unsigned long *)sys_mycall;  //将335号指向自己写的调用函数
	make_ro((unsigned long)sys_call_t1);  
		setback_cr0(val);
	return 0;  
}

static void __exit syscall_cleanup_module(void){  
	printk("Module syscall unloaded.");  

	make_rw((unsigned long)sys_call_t1);  
	sys_call_t1[335] = (unsigned long *) orig_saved ;   
	make_ro((unsigned long)sys_call_t1);  
}
module_init(syscall_init_module);  
module_exit(syscall_cleanup_module);  
  
MODULE_LICENSE("GPL");  
MODULE_DESCRIPTION("mysyscall");  
 
  
 
