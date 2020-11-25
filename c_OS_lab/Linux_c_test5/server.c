#include <stdio.h>
#include <stdlib.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/msg.h>

#define MSGKEY 77

struct msgbuf {
    long mtype;
    char mtext[1024];
}msg;

void server(){
    int msgqid=msgget(MSGKEY,IPC_CREAT|0777);
    do{
        msgrcv(msgqid,&msg,1024,0,0);
        printf("server receive message\n");
    }while (msg.mtype!=10);
    msgctl(msgqid,IPC_RMID,0);
    exit(0);
}

int main(){
    server();
    return 0;
}