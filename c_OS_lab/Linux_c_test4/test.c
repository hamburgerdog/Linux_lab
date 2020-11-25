#include <stdio.h>
#include <stdlib.h>
#include <zconf.h>
#include <signal.h>

/*
 *  编写程序：用fork( )创建两个子进程，再用系统调用signal( )让父进程捕捉键盘上来的中断信号（即按^c键）；
 *  捕捉到中断信号后，父进程用系统调用kill( )向两个子进程发出信号，子进程捕捉到信号后分别输出下列信息后终止：
 *  Child process1 is killed by parent!
 *  Child process2 is killed by parent!
 *  父进程等待两个子进程终止后，输出如下的信息后终止：
 *  Parent process is killed!
 *  分析利用信号量机制中的软中断通信实现进程同步的机理。
 */

//  信号处理函数
void breakFunc();
void showMsg1();
void showMsg2();

//  全局标识符
int pid_1,pid_2;

int main() {
    //  进程1
    pid_1=fork();
    if (pid_1<0){
        return -1;
    } else if (pid_1==0){
        //  终止进程 忽略信号
        signal(SIGINT,SIG_IGN);
        //  接受父进程的信号并调用相关处理函数
        signal(SIGUSR1,showMsg1);
        pause();
    }
    //  父进程与进程2
    else{
        //  进程2
        pid_2=fork();
        if (pid_2<0){
            return -1;
        } else if (pid_2==0){
            signal(SIGINT,SIG_IGN);
            signal(SIGUSR2,showMsg2);
            pause();
        }
        //  父进程
        else{
            //  父进程接受中断信号
            signal(SIGINT,breakFunc);
            //  等待两个子进程结束
            wait(0);
            wait(0);
            printf("Parent process is killed!\n");
            exit(0);
        }
    }
    return 0;
}

//  实现父进程接受中断信号时调用的函数
void breakFunc(){
    printf("\n");
    kill(pid_1,SIGUSR1);    //  SIGUSR? 表示用户自我定义的信号
    kill(pid_2,SIGUSR2);
}

//  实现子进程接受父进程的信号时调用的函数
void showMsg1(){
    printf("Child process1 is killed by parent!\n");
    exit(0);
}
void showMsg2(){
    printf("Child process2 is killed by parent!\n");
    exit(0);
}