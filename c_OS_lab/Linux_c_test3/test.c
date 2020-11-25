#include <stdio.h>
#include <stdlib.h>
#include <zconf.h>

/*
 * 编写一段多进程并发运行的程序，用lockf( )来给每一个进程加锁，以实现进程之间的互斥，观察并分析出现的现象及原因。
 */


int main() {
    int pid_1,pid_2;
    //  进程1
    pid_1=fork();
    if(pid_1 < 0 ){
        printf("Fork process 1 error\n");
        return -1;
    } else if (pid_1==0){
        /*
         * lockf(int,int,int)
         * 第一个参数1表示对stdout的加锁
         * 第二个参数的1表示加锁 0表示解锁
         * 第三个参数0表示从文件开始到结尾
         */
        lockf(1,1,0);
        for (int i = 0; i < 10; ++i) {
            printf("This is process 1\n");
        }
        lockf(1,0,0);
    } 
    //  父进程
    else{
        //  子进程2
        pid_2 = fork();
        if (pid_2 < 0 ){
            printf("Fork process 2 error\n");
            return -1;
        } else if (pid_2 == 0){
            lockf(1,1,0);
            for (int i = 0; i <10; ++i) {
                printf("This is process 2\n");
            }
            lockf(1,0,0);
        } 
        //  父进程
        else {
            lockf(1,1,0);
            for (int i = 0; i < 10; ++i) {
                printf("This is father process\n");
            }
            lockf(1,0,0);
        }
    }

    return 0;
}