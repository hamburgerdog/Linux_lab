#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/sched.h>  


unsigned long timeout=500*HZ;
static int __init hello_init(void){
        printk(KERN_INFO "process begin running!\n");
	__set_current_state(TASK_INTERRUPTIBLE);
    	timeout=schedule_timeout(timeout);
	printk(KERN_INFO "process restart to run after:%ld !\n",timeout);
        return 0;
}

static void __exit hello_exit(void){
        printk(KERN_INFO "Unloading....\n");
}

module_init(hello_init);
module_exit(hello_exit);
