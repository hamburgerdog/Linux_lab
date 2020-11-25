#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>

void doThread1(sem_t *sem){
    for (int i = 0; i < 10; ++i) {
        sem_wait(sem);
        printf("doThread1\n");
        sem_post(sem);
    }

}
void doThread2(sem_t *sem){
    for (int i = 0; i < 10; ++i) {
        sem_wait(sem);
        printf("doThread2\n");
        sem_post(sem);
    }
}
int main(){
    pthread_t pid1,pid2;
    sem_t * sem=sem_open("mythread1",O_CREAT, S_IRUSR | S_IWUSR | S_IXUSR,1);
    pthread_create(&pid1,NULL,(void *)doThread1,sem);
    pthread_create(&pid2,NULL,(void *)doThread2,sem);
    pthread_join(pid1,NULL);
    sem_close(sem);
    sem_unlink("mythread1");
    return 0;
}