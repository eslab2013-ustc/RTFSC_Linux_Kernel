#include<stdio.h>
#include<sys/types.h>
#include<errno.h>
#include<sys/ipc.h>
#include<sys/sem.h>
#include<string.h>

#define SEMKEY 4567L
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
    
    int res=0;

    res=semctl(semid,0,GETVAL);
    printf("1、parent  semval===%d\n",res);

    if(fork()==0)
    {
        
         res=semctl(semid,0,IPC_RMID);
         printf("Delete semaphore result===%d\n",res);
         exit(0);
    }
    sleep(1);
    res=semctl(semid,0,GETVAL);
    printf("2、parent  semval===%d\n error:%s\n",res,strerror(errno));
    
    return 0;
}
