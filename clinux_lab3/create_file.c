#include <linux/module.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>

static struct proc_dir_entry *proc_parent;
int len,temp;
char *msg;

static ssize_t read_proc(struct file *filp,char __user *buf,size_t count,loff_t *offp){
	if(count>temp)
		count=temp;
	temp=temp-count;

	copy_to_user(buf,msg,count);
	
	if(count==0)
		temp=len;
	return count;
}

static const struct file_operations proc_fops = {
	.read		= read_proc
};

void create_new_proc_entry(void){
	proc_parent = proc_mkdir("hello",NULL);
	if(proc_parent==NULL){
		printk(KERN_INFO "Error create proc entry\n");
	}
	
	proc_create("world",0,proc_parent,&proc_fops);

	msg="hello,world!\n";
	len=strlen(msg);
	temp=len;
	printk(KERN_INFO "1.len=%d",len);
	printk(KERN_INFO "proc initalized\n");
}

int __init proc_init(void){
	create_new_proc_entry();
	return 0;
}

void __exit proc_cleanup(void){
	printk(KERN_INFO "Clean up module\n");
	remove_proc_entry("hello",proc_parent);
	remove_proc_entry("world",NULL);
}

MODULE_LICENSE("GPL");
module_init(proc_init);
module_exit(proc_cleanup);
