#include <stdio.h>
#include <stdlib.h>
#include <zconf.h>
#include <sys/mman.h>

//  SUM(1,2,3,...,N);
#define N 50

int main(){
    //  创建共享匿名虚存区
    void *start = 0;
    int size = 4;
    int fd = 0;
    int fd_offset = 0;
    int *result_prt = mmap(start,size,PROT_READ|PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS,fd,fd_offset);

    //  创建进程
    int pid =fork();
    //  异常处理
    if(pid<0){
        printf("create process error!!\n");
    }
    //  子进程
    if(pid==0){
        int sum=0;
        for(int i=1;i<=N;i++)
            sum+=i;
        *result_prt=sum;
        _Exit(1);
    }
    //  父进程
    else{
        wait(0);
        printf("[1+2+3+...+%d]=%d\n",N,*result_prt);
        exit(0);
    }
}


