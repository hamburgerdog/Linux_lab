// 利用realloc函数来修改上述malloc函数申请的动态内存大小
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char * upcast(char *,char *);
char * capixj(char *,char *);

int main(){
    char *str=NULL;
    str=upcast("Hello,World!",str);
    printf("%s\n",str);
    str=capixj("MY NAME IS JOSIAHBOT",str);
    printf("%s\n",str);

    free(str);

    return 0;
}

char *upcast(char *inputstring,char *addr){
    int inputstring_length=strlen(inputstring);

    if(addr==NULL)
        addr=realloc(NULL,inputstring_length+1);
    else
        addr=realloc(addr,inputstring_length+1);

    if(addr==NULL){
        printf("create memory error !!\n");
        exit(-1);
    }

    strcpy(addr,inputstring);

    for (int i = 0; i < inputstring_length; ++i) {
        if (addr[i]>=97&&addr[i]<=122){
            addr[i]-=32;
        }
    }
    return addr;
}

char *capixj(char *inputstring,char *addr){
    int inputstring_length=strlen(inputstring);

    if(addr==NULL)
        addr=realloc(NULL,inputstring_length+1);
    else
        addr=realloc(addr,inputstring_length+1);

    if(addr==NULL){
        printf("create memory error !!\n");
        exit(-1);
    }

    strcpy(addr,inputstring);

    for (int i = 0; i < inputstring_length; ++i) {
        if (addr[i]>=65&&addr[i]<=90){
            addr[i]+=32;
        }
    }

    return addr;
}

