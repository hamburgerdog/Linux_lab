#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

int main(){
    // 进程标识符
    int pid_1,pid_2;
    // 一个存放输出的字符串和一个存放输出的字符串
    char opipe[100],ipipe[100];
    // fd[0]表示可读 fd[1]表示可写
    int fd[2];
    // 创建管道程序
    pipe(fd);

    pid_1=fork();
    switch(pid_1){
        // 异常处理
        case -1:
            printf("create process 1 error\n");
            return -1;
        // 子进程1 向管道发送数据
        case 0:
            // 资源锁 避免其他进程在本进程写时再进行操作
            lockf(fd[1],1,0);
            // 写到字符串
            sprintf(opipe,"Child 1 is sending a message!");
            // 发送到管道
            write(fd[1],opipe,50);
            // 避免同时占用
            sleep(5);
            // 解锁
            lockf(fd[1],0,0);
            exit(0);
        // 父进程与子进程2
        default:
            pid_2=fork();
            switch (pid_2){
                case -1:
                    printf("create process 2 error\n");
                    return -1;
                // 子进程2
                case 0:
                    lockf(fd[1],1,0);
                    sprintf(opipe,"Child 2 is sending a message!");
                    write(fd[1],opipe,50);
                    sleep(5);
                    lockf(fd[1],0,0);
                    exit(0);
                // 父进程
                default:
                    wait(0);
                    // 父进程读管道信息
                    read(fd[0],ipipe,50);
                    printf("main process read message1:\t%s\n",ipipe);
                    wait(0);
                    read(fd[0],ipipe,50);
                    printf("main process read message2:\t%s\n",ipipe);
                    exit(0);
            }
    }
}