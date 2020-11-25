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

void client(){
    int msgqid = msgget(MSGKEY,IPC_CREAT|0777);
    for(int i = 1;i<=10;i++){
        msg.mtype=i;
        printf("client send message\n");
        msgsnd(msgqid,&msg,1024,0);
    }
    exit(0);
}

int main(){
    client();
    return 0;
}