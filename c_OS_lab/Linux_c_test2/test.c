#include <stdio.h>
#include <stdlib.h>
#include <zconf.h>
#include <sys/wait.h>


/*
 * 用fork( )创建一个进程，再调用exec( )，用新的程序替换该子进程的内容，
 * 利用wait( )来控制进程执行顺序，掌握进程的睡眠、同步、撤消等进程控制方法，
 * 并根据实验结果分析原因。
 */


int main() {
    int pid = fork();       //  子进程1标识符

    if(pid <0 ){
        printf("Fork child process error\n");
        return -1;
    }
    //  子进程1
    else if (pid == 0){
        printf("This is child process\n");
        //  调用环境变量中的 ls 文件,参数为 'ls'且以空字符指针表示结尾
        execlp("ls","ls",(char*)0);
    }
    //  父进程
    else{
        printf("This is father\n");

        //  等待子进程1执行完毕 实现父进程睡眠
        printf("Wait for child process\n");
        int status;
        wait(&status);
        printf("Father process continue\n");

        int pid_2 = fork();     //  创建子进程2
        if (pid_2<0){
            printf("Fork child process 2 error\n");
            return -1;
        }
        //  子进程2
        else if (pid_2 == 0){
            printf("This is child process2\n");
            //退出  0表示正常退出 -1表示异常退出
            exit(0);

            /*以下代码在进程退出后并无实际意义,永远得不到执行*/
            printf("CHILD PROCESS SAY HELLO WORLD");
        }
        //  父进程
        else{
            //  等待子进程执行
            wait(&status);
            printf("Welcome back to father\n");
        }
    }

    return 0;
}