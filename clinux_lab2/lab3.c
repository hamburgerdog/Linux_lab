/*
利用链表结构，malloc函数和free函数实现将终端输入的一系列字符串用链表形式保存下来
然后再将这些数据组装起来，回显到显示屏
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//  每次输入的字符数大小
#define DATA_SIZE 10

//  链表数据结构
typedef struct stringdata{
    char *str;
    struct stringdata *next
}mystr;

char *add_node(mystr *,char *);
void freedata(char *);
void display(char *);

/*
    ======================使用须知=======================
    请先在程序中display()函数打入断点，并在调试模式下查看输出结果
    直接运行时使用 ctrl+D 指令会造成程序直接退出,无法看到输出!!!
    ====================================================
*/

int main(){
    char input[DATA_SIZE];
    mystr *start = NULL;
    //  获取字符串,用ctrl+D结束
    while(fgets(input, sizeof(input),stdin)){
        start=add_node(start,input);
    }
    display(start);
    freedata(start);
    return 0;
}
//  链表添加节点
char *add_node(mystr *start,char *s){
    //  新创节点,用于存放当前输入的字符串
    mystr *new = malloc(sizeof(mystr)+1);
    if(new==NULL){
        printf("create memory error !!!\n");
        exit(-1);
    }
    //  先为字符串动态分配内存,再将地址s中的数据拷贝到该内存中
    new->str=malloc(strlen(s)+1);
    if(new->str==NULL){
        printf("create memory error !!!\n");
        exit(-1);
    }
    //  拷贝字符串
    strcpy(new->str,s);
    new->next=NULL;
    
    //  如果链表为空 则 直接将新增节点作为头节点
    if(start==NULL){
        start=new;
        return start;
    }

    //  如果链表不为空 则 找到链表中最后一个节点
    mystr *ret=start;
    //  使用临时节点是为了避免覆盖链表中的数据
    mystr *temp=NULL;
    while(ret->next!=NULL){
        temp=ret->next;
        ret=temp;
    }
    ret->next=new;

    return start;
}
//  展示链表中的数据
void display(char *start){
    //  异常处理
    if(start==NULL){
        printf("ERROR: stringdata is NULL!!\n");
        exit(-1);
    }
    
    mystr *cur=start;
    mystr *temp=NULL;
    while(cur!=NULL){
        printf("%s",cur->str);
        temp=cur->next;
        cur=temp;
    }
}
//  释放资源
void freedata(char *start){
    mystr *ret=start;
    mystr *temp=NULL;
    while(ret!=NULL){
        temp=ret->next;
        free(ret->str);
        free(ret);
        ret=temp;
    }
}
