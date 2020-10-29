#include <stdio.h>
#include <stdlib.h>
#include <zconf.h>


/*
 * 当此程序运行时，在系统中有一个父进程和两个子进程并发执行
 */


int main() {
    int child_pid_1,child_pid_2;

    // fork之前只有一个进程在执行代码,fork后有两个进程执行
    child_pid_1=fork();
    if (child_pid_1<0){
        printf("fork child1 error\n");
        return -1;
    }
    // 子进程1
    else if (child_pid_1==0){
        printf("This is child1\tPID:%d\n",getpid());
        sleep(5);
    }
    // 父进程
    else {
        child_pid_2=fork();
        if (child_pid_2<0){
            printf("fork child2 error\n");
            return -1;
        }
        // 子进程2
        else if (child_pid_2==0){
            printf("This is child2\tPID:%d\n",getpid());
            sleep(5);
        }
        // 父进程
        else{
            printf("This is father\tPID:%d\n",getpid());
            wait(0);
            wait(0);
        }

    }

    return 0;
}