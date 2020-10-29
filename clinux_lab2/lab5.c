#include<linux/module.h>
#include<linux/init.h>
#include<linux/interrupt.h>
#include<linux/sched.h>

//  需读取的进程号
static int pid;
//  用于在加载模块时，读取pid
module_param(pid,int,0644);
static int __init memtest_init(void)
{
    // linux下的进程结构
    struct task_struct *p;
    // linux下的虚存区结构
    struct vm_area_struct *temp;
    printk("The virtual memory areas(VMA) are:\n");
    // find_task_by_vpid() 已经不适用于查找进程了
    p= pid_task(find_vpid(pid),PIDTYPE_PID);
    temp = p->mm->mmap;
    while(temp)
    {
        // printk("start:%p\tend:%p\n",(unsigned long *)temp->vm_start,(unsigned long *)temp->vm_end);
        // 修改输出的格式 便于观察
        printk("start:%lx\tend:%lx\n",(unsigned long *)temp->vm_start,(unsigned long *)temp->vm_end);
        temp=temp->vm_next;
    }
    return 0;
}
// 退出
static void __exit memtest_exit(void)
{
    printk("Unloading my module.\n");
    return;
}
module_init(memtest_init);
module_exit(memtest_exit);
// 使用GPL协议开发
MODULE_LICENSE("GPL");
