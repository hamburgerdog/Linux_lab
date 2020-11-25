#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

// 进程执行的函数
void thread(){
    system("ps");
    for (int i = 0; i < 3; ++i) {
        printf("This is a pthread\n");
    }
}

int main(){
    pthread_t pid;  //  子线程标识符
    //  创建子进程 返回0表示创建线程成功
    int ret = pthread_create(&pid,NULL,(void *)thread,NULL);
    if (ret!=0){
        printf("created pthread error \n");
        return -1;
    }
    //  等待线程执行完毕再继续执行
    pthread_join(pid,NULL);
    
    printf("main process continue\n");

    return 0;
}
