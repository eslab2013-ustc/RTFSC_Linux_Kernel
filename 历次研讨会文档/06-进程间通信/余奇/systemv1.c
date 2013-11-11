#include<stdio.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/sem.h>

#define SEMKEY 1234L
#define PERMS 0666

struct sembuf op_down[1]={0,-1,0};
struct sembuf op_up[1]={0,1,0,};

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
        printf("semctl result:%d\n",res);
    }//endif
}

void down(){
    res=semop(semid,&op_down[0],1);
    printf("down result:%d\n",res);
}

void up(){
    res=semop(semid,&op_up[0],1);
    printf("up result:%d\n",res);
}

int main(){
    init_sem();
    
    printf("Before critical code\n");
    down();
    
    printf("In critical code\n");
    sleep(5);
    up();
    printf("After critical code\n");
    return 0;
}
