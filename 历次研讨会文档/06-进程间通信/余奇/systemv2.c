#include<stdio.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/sem.h>
#include<pthread.h>

#define SEMKEY 2345L
#define PERMS 0666

struct sembuf op_down[1]={0,-1,0};
struct sembuf op_up[1]={0,1,0};
/*
struct count{
   int producecount;
}
*/
int semid =-1;
int res;

void init_sem(){
    semid=semget(SEMKEY,0,IPC_CREAT|PERMS);
    printf("semid============%d\n",semid);
    if(semid<0)
    {
        printf("Create the semaphore\n");

        semid=semget(SEMKEY,1,IPC_CREAT|PERMS);
        if(semid<0)
        {
            printf("Couldn't create semaphore!\n");
            return;
        }//endif
        printf("new semid============%d\n",semid);
        res=semctl(semid,0,SETVAL,1);
    }//endif
}

void down(){
    res=semop(semid,&op_down[0],1);
}

void up(){
    res=semop(semid,&op_up[0],1);
}

void * processv(void *arg)
{
    int num=*(int *)arg;
    
    printf("Before critical code-----process(%d)\n",num);
    down();
    
    printf("In critical code-----process(%d)\n",num);
    sleep(10);
    up();

    printf("After critical code-----process(%d)\n",num);
}

int main(){
    init_sem();
    
    pthread_t tid1,tid2;
     
    int num1=1,num2=2;
     

    pthread_create(&tid1,NULL,processv,&num1);
    sleep(1);
    pthread_create(&tid2,NULL,processv,&num2);

    pthread_join(tid1,NULL);
    pthread_join(tid2,NULL);

    return 0;
}
