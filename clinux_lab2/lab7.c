#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <fcntl.h>


int main(){
    //  以只读模式打开一个文件
    int fd = open("/Users/HONGmacbook/Desktop/Linux_lab3/test_data",O_RDONLY);
    if(fd<0){
        printf("open file error !!\n");
        exit(1);
    }
    //  读取文件创建一个虚存区
    void *start = 0;
    int size = 12;
    char *buf = (char *)mmap(start,size,PROT_READ,MAP_PRIVATE,fd,0);
    //  输出虚存区
    for (int i = 0; i < strlen(buf); ++i) {
        printf("%c\t",buf[i]);
    }
    return 0;
}


