#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/shm.h>

#define SHMSIZE 1024
#define SHMKEY 77

struct shmbuf{
    int written;
    char text[SHMSIZE];
};

int main(){
    void *shmaddr = NULL;
    struct shmbuf *shareBuf = NULL;
    char buffer[SHMSIZE+1];

    int shmid = shmget(SHMKEY, sizeof(struct shmbuf),0777|IPC_CREAT);
    shmaddr = shmat(shmid,(void *)0,0);
    shareBuf = (struct shmbuf *)shmaddr;
    int i = 0;
    while(1){
        while (shareBuf->written==1){
            sleep(1);
        }
        printf("请输入一些数据:\n");
        fgets(buffer,SHMSIZE,stdin);
        strncpy(shareBuf->text,buffer,SHMSIZE);

        shareBuf->written=1;
        if (strncmp(buffer,"end",3)==0){
            break;
        }
    }
    shmdt(shmaddr);

    exit(0);

}