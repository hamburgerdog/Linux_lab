#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>

static int hello_init(void){
        printk(KERN_INFO "HELLO WORLD! I am XJOSIAH ! \n");
        return 0;
}

static void hello_exit(void){
        printk(KERN_INFO "GOODBYE WORLD! Remeber me XJOSIAH\n");
}

module_init(hello_init);
module_exit(hello_exit);
