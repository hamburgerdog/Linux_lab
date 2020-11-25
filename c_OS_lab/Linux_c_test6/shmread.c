#include <stddef.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define SHMSIZE 1024
#define SHMKEY 77

struct shmbuf{
    int written;
    char text[SHMSIZE];
};

int main(){
    void *shmaddr = NULL;
    struct shmbuf *shareBuf = NULL;
    int shmid = shmget(SHMKEY, sizeof(struct shmbuf),IPC_CREAT|0777);
    shmaddr = shmat(shmid,0,0);
    shareBuf = (struct shmbuf *)shmaddr;
    shareBuf->written=0;
    while(1){
        if (shareBuf->written==1){
            printf("read shm :%s",shareBuf->text);
            sleep(1);
            shareBuf->written=0;
            if (strncmp(shareBuf->text,"end",3)==0){
                break;
            }
        } else sleep(1);
    }
    shmdt(shmaddr);
    shmctl(shmid,IPC_RMID,0);
    exit(0);
}