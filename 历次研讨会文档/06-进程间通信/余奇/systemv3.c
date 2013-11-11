#include<stdio.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/sem.h>

#define SEMKEY 3456L
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
        res=semctl(semid,0,SETVAL,4);
        //printf("semctl result:%d\n",res);
    }//endif
}

void down(){
    res=semop(semid,&op_down[0],1);
}

void up(){
    res=semop(semid,&op_up[0],1);
}

int main(){
    init_sem();
    
    int semval=0;

    semval=semctl(semid,0,GETVAL);
    printf("1、parent  semval===%d\n",semval);

    if(fork()==0)
    {
         down();
         semval=semctl(semid,0,GETVAL);
         printf("child  semval===%d\n",semval);
         exit(0);
    }
    sleep(1);
    semval=semctl(semid,0,GETVAL);
    printf("2、parent  semval===%d\n",semval);
    
    return 0;
}
