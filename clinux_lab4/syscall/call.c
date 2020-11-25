#include <stdio.h>
#include <linux/kernel.h>
#include <sys/syscall.h>
#include <unistd.h>

int main(int argc,char **argv){
	long int a=syscall(335);
	printf("System call my_sycall return %ld\n",a);
	return 0;
}

