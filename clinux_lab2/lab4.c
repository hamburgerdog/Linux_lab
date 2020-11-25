#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(){
    //  动态分配一个1024B大小的char类型内存空间
    unsigned char *buff;
    buff=(char *)malloc(sizeof(char)*1024);
    //  异常避免
    if(buff==NULL){
        printf("create memory error!!\n");
        exit(-1);
    }
    printf("My pid is %d\n",getpid());
    //  长期阻塞方便观察
    //  /proc/PID/maps 存放有进程的虚存使用信息 通过这个可以直接观察
    for (int i = 0; i < 60; ++i) {
        sleep(60);
    }
    return 0;
}


