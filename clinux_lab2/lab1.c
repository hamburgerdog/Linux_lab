// 利用malloc函数和free函数来实现动态内存的申请和释放
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *upcast(char *);
char *capixj(char *);

int main() {
    char *str=upcast("Hello,world!\tmy NAME is JOSIAHBOT");
    printf("This is upcast() :\t%s\n",str);

    //  要输入的字符串相同可以重新使用相同地址空间
    str=capixj("Hello,world!\tmy NAME is JOSIAHBOT");
    printf("This is capixj() :\t%s\n",str);

    free(str);

    return 0;
}

//  全部大写
char *upcast(char *inputstring){
    char *str;
    int inputstring_length = strlen(inputstring);

    str=malloc(inputstring_length+1);

    //  异常处理:创建内存地址空间失败则直接退出
    if (str==NULL){
        printf("create memory error!!\n");
        exit(-1);
    }

    //  字符串复制
    strcpy(str,inputstring);

    for (int i = 0; i < inputstring_length; ++i) {
        if (str[i]>=97&&str[i]<=122){
            str[i]-=32;
        }
    }

    return str;
}

//  全部小写
char *capixj(char *inputstring){
    char *str;
    int inputstring_length = strlen(inputstring);

    str=malloc(inputstring_length+1);

    if (str==NULL){
        printf("create memory error!!\n");
        exit(-1);
    }

    strcpy(str,inputstring);

    for (int i = 0; i < inputstring_length; ++i) {
        if (str[i]>=65&&str[i]<=90){
            str[i]+=32;
        }
    }

    return str;
}
